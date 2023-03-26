#pragma once

#include "bios.h"
#include "mem.h"
#include "types.h"

struct Interconnect {
	BIOS* BIOS;
	RAM* RAM;
};

typedef struct Interconnect Interconnect;
Interconnect* new_interconnect(BIOS*, RAM*);
u32 INTER_load32(Interconnect*, u32);
void INTER_store32(Interconnect*, u32, u32);
