#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interconnect.h"
#include "cpu.h"
#include "types.h"
#include "defs.h"

CPU*
new_cpu(Interconnect* inter) {
	CPU* cpu = malloc(sizeof(CPU));
	cpu->PC = 0xBFC00000;
	cpu->SR = 0x0;
	cpu->INTER = inter;
	cpu->LOAD.IDX = 0; cpu->LOAD.VAL = 0;
	cpu->next_instruction = new_instr(0x0); /* NOP */
	memset(cpu->REGS, 0, sizeof(cpu->REGS));
	memset(cpu->OUT_REGS, 0, sizeof(cpu->OUT_REGS));
	return cpu;
}

u32
CPU_load32(CPU* cpu, u32 addr)
{
	return INTER_load32(cpu->INTER, addr);
}

void
CPU_decode_and_execute(CPU* cpu, Instruction* i)
{
	switch(i->fn) {

	/* LUI */
	case 0xF:
		cpu->OUT_REGS[i->rt] = i->imm << 16;
		break;

	/* ORI */
	case 0xD:
		cpu->OUT_REGS[i->rt] = cpu->REGS[i->rs] | i->imm;
		break;

	/* SW */
	case 0x2B: {
		u32 addr = 0;
		u32 val = 0;

		if ((cpu->SR & 0x10000) != 0) {
			fprintf(stdout, "Ignoring store while cache is isolated\n");
			break;
		}

		addr = cpu->OUT_REGS[i->rs] + (i32)IMM_SE(i->imm);
		val = cpu->OUT_REGS[i->rt];
		INTER_store32(cpu->INTER, addr, val);
		break;
	}
	case 0x0:
		/* Secondary Function */
		switch(i->sub) {
		/* SLL - Shift Left Logical */
		case 0x0:
			cpu->OUT_REGS[i->rd] = cpu->OUT_REGS[i->rt] << i->shift;
			break;
		/* OR */
		case 0x25:
			cpu->OUT_REGS[i->rd] = cpu->OUT_REGS[i->rs] | cpu->OUT_REGS[i->rt];
			break;
		default:
			fprintf(stdout, "Unhandled instruction %08X\n", i->fn);
			break;
		}
		break;
	/* ADDIU */
	case 0x9:
		cpu->OUT_REGS[i->rt] = cpu->OUT_REGS[i->rs] + IMM_SE(i->imm);
		break;
	case 0x2:
		cpu->PC = (cpu->PC & 0xf0000000) | (IMM_JUMP(i->instr) << 2);
		fprintf(stderr, "THIS IS A JUMP/n");
		print_instr(i);
		//fprintf(stdout, "\n\n%08x\n\n", cpu->PC);
		exit(EXIT_FAILURE);
		break;
	/* Coprocessor 0 opcodes */
	case 0x10: {
		u8 cpu_r = i->rt;
		u8 cop_r = i->rd;
		u8 v = cpu->OUT_REGS[cpu_r];

		switch (cop_r) {
		case 3: case 5: case 6: case 7: case 9: case 11:
			if (v != 0) {
				fprintf(stderr, "Unhandled write to cop0r%08X\n", cop_r);
				exit(EXIT_FAILURE);
			}
			break;
		case 12:
			cpu->SR = v;
			break;
		case 13: /* Cause Register */
			if (v != 0) {
				fprintf(stderr, "Unhandled write to CAUSE register\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr,"Unhandled cop0 register %08X\n", cop_r);
			exit(EXIT_FAILURE);
		}
		break;
	}
	/* BNE */
	case 0x5: {
		u32 offset;
		if (cpu->OUT_REGS[i->rs] != cpu->OUT_REGS[i->rt]) {
			offset = (i32)IMM_SE(i->imm);
			/* Compensation for hardcoded +4 in CPU_run_next_instruction */
			offset <<= 2;
			cpu->PC += offset;
			cpu->PC -= 4;
		}
		break;
	}
	/* ADDI */
	case 0x8:
		/* This needs work. Find a way to know if an overflow in unsigned
		   arithmetics occured. Don't know the behaviour now. Prob an overflow crash. */
		cpu->OUT_REGS[i->rt] = cpu->OUT_REGS[i->rs] + (i32)IMM_SE(i->imm);
		break;
	/* LW */
	case 0x23: {
		u32 addr;
		u32 val;

		if ((cpu->SR & 0x10000) != 0) {
			fprintf(stdout, "Ignoring load while cache is isolated\n");
			break;
		}
		addr = cpu->OUT_REGS[i->rs] + IMM_SE(i->imm);
		val = CPU_load32(cpu, addr);
		cpu->LOAD.IDX = cpu->REGS[i->rt]; cpu->LOAD.VAL = val;
		/*cpu->OUT_REGS[i->rt] = val;*/
		break;
	}

	default:
		/*fprintf(stdout, "\n\n%08x\n\n", i->instr);*/
		print_instr(i);
		free(cpu);
		exit(EXIT_FAILURE);
		break;
	}

	/* AFTER 5-STAGE PIPELINGING (IF ID EX MEM WB) */

	/* Enforce 0 in $ZERO */
	cpu->OUT_REGS[0] = 0;
}

void
CPU_run_next_instruction(CPU* cpu)
{
	u32 pc = cpu->PC;
	u32 reg_idx = 0;
	u32 val = 0;

	Instruction *instruction = cpu->next_instruction;
	cpu->next_instruction = new_instr(CPU_load32(cpu, pc));
	fprintf(stdout, "\nFULL INSTRUCTION %X\n", instruction->instr);
	fprintf(stdout, "CURRENT INSTRUCTION: %X - SUB INSTRUCTION %X\n", instruction->fn, instruction->sub);
	cpu->PC+=4;

	reg_idx = cpu->LOAD.IDX;
	val = cpu->LOAD.VAL;
	cpu->OUT_REGS[reg_idx] = val;
	/* Reset the load*/
	cpu->LOAD.IDX = 0; cpu->LOAD.VAL = 0;

	CPU_decode_and_execute(cpu, instruction);

	/* Copy back to the registers */
	memcpy(cpu->REGS, cpu->OUT_REGS, sizeof(u32) * 32);
}

Instruction*
new_instr(u32 instruction)
{
	Instruction* ins = malloc(sizeof(Instruction));
	ins->instr = instruction;
	ins->fn = instruction >> 26;
	ins->sub = instruction & 0x3f;
	ins->rs = (instruction >> 21) & 0x1f;
	ins->rt = (instruction >> 16) & 0x1f;
	ins->rd = (instruction >> 11) & 0x1f;
	ins->shift = (instruction >> 6) & 0x1f;
	ins->imm = instruction & 0xffff;
	print_instr(ins);
	return ins;
}

void
print_instr(Instruction *ins)
{
	fprintf(stdout, "\n");
	fprintf(stdout,"FUNCTION: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->fn));
	fprintf(stdout,"SUBFUNCTION: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->sub));
	fprintf(stdout,"SOURCE REGISTER: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->rs));
	fprintf(stdout,"OPERAND REGISTER: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->rt));
	fprintf(stdout,"DESTINATION REGISTER: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->rd));
	fprintf(stdout,"IMMEDIATE: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ins->imm));
	fprintf(stdout, "\n");
}
