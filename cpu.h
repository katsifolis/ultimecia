#pragma once

#include "interconnect.h"
#include "types.h"

#define IMM_JUMP(i) (i & 0x3ffffff)
#define IMM_SE(i) (((u32)((i16)(i & 0xffff))));

struct Instruction {
	u32 instr;
	u32 fn;
	u32 rt;
	u32 rs;
	u32 rd;
	u32 imm;
	u32 sub;
	u32 shift;
};

typedef struct Instruction Instruction;

struct Load {
	u8 IDX; 
	u32 VAL;
};

typedef struct Load Load;

struct CPU {

	/* Program Counter register */
	u32 PC;
	
	/* Cop0 register 12: Status Register*/
	u32 SR;

	Instruction* next_instruction;

	Interconnect* INTER;

	/* General Registers */
	u32 REGS[32];

	/* Second set of regs to emulate the LOAD DELAY SLOT.
	   They contain the output of the current instruction */
	u32 OUT_REGS[32];

	/* LOAD -> on load delay slots */
	Load LOAD;

};

typedef struct CPU CPU;

CPU* new_cpu(Interconnect*);
u32  CPU_load32(CPU*, u32);
void CPU_decode_and_execute(CPU*, Instruction*);
void CPU_run_next_instruction(CPU*);

Instruction* new_instr(u32);
void print_instr(Instruction*);
