#include <stdio.h>
#include <stdlib.h>

#include "interconnect.h"
#include "bios.h"
#include "mem.h"
#include "util.h"
#include "types.h"
#include "defs.h"

Interconnect*
new_interconnect(BIOS* bios, RAM* ram) {
	Interconnect *inter = malloc(sizeof(Interconnect));
	inter->BIOS = bios;
	inter->RAM = ram;
	return inter;
}

u32
INTER_load32(Interconnect* inter, u32 addr)
{
	u32 offset = 0;

	if (addr % 4 != 0) 
		fprintf(stderr, "Unaligned_load32_address: %08X", addr);
   	
	/* Assert address Mappings */

	if (UTIL_contains(BIOS_RANGE, addr)) {
		offset = addr - BIOS_START;
		return BIOS_load32(inter->BIOS, offset);
	} else if (UTIL_contains(RAM_RANGE, addr)) {
		offset = addr - RAM_START;
		return RAM_load32(inter->RAM, offset);
	} else {
		fprintf(stderr, "Unhandled Load32 At Address %08X", addr);
		exit(EXIT_FAILURE);
	}

}

void
INTER_store32(Interconnect* inter, u32 addr, u32 val)
{
	u32 offset = 0;

	if (addr % 4 != 0) 
		fprintf(stderr, "Unaligned_store32_address: %08X", addr);

	/* Ignore RAM */
	if (UTIL_contains(RAM_RANGE, addr)) {
		fprintf(stdout, "Ignoring RAM write; %X\n", addr);
		return;
	}

	/* Ignore RAM_SIZE register */
	if (UTIL_contains(RAM_SIZE_RANGE, addr)) {
		fprintf(stdout, "Ignoring RAM_SIZE register write; %X\n", addr);
		return;
	}

	/* Ignore CACHE_CONTROL address */
	if (UTIL_contains(CACHECONTROL_RANGE, addr)) {
		fprintf(stdout, "Ignoring CACHECONTROL address write: %X\n", addr);
		return;
	}

	if (UTIL_contains(MEMCONTROL_RANGE, addr)) {
		offset = addr - MEMCONTROL_START;

		/* sry for the magic values */
		switch(offset) {
		case 0:
			if (val != 0x1F000000) {
				fprintf(stderr, "Bad Expansion 1 base address: %08X", val);
				exit(EXIT_FAILURE);
			}
			break;
		case 4:
			if (val != 0x1F802000) {
				fprintf(stderr, "Bad expansion 2 base address: %08X", val);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr, "Unhandled write to MEMCONTROL register\n");
			return;
		}
	} 
   	
	fprintf(stderr, "Unhandled store 32 into address: %08X\n", addr);
}
