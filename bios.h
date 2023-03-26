#pragma once

#include "types.h"

struct BIOS {
	u8* data;
};

typedef struct BIOS BIOS;

BIOS* new_bios(char*);
u32 BIOS_load32(BIOS*, u32);
