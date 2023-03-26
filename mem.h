#pragma once

#include "util.h"
#include "types.h"

struct RAM {
	u8 *data;
};

typedef struct RAM RAM;

RAM* new_ram();
u32 RAM_load32(RAM*, u32);
void RAM_store32(RAM* r, u32, u32);
