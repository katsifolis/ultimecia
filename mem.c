#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "types.h"
#include "util.h"

RAM*
new_ram() 
{
	RAM *ram = malloc(sizeof(RAM));
	ram->data = malloc(sizeof(u8) * RAM_SIZE);
	memset(ram->data, 0xca, RAM_SIZE);
	return ram;
}

u32
RAM_load32(RAM* r, u32 offset)
{
	return (r->data[offset + 3] << 24) | 
		   (r->data[offset + 2] << 16) | 
		   (r->data[offset + 1] <<  8) | 
		   (r->data[offset + 0]);
}

void
RAM_store32(RAM* r, u32 offset, u32 val)
{
	u8 b0 = val;
	u8 b1 = (val >> 8); 
	u8 b2 = (val >> 16);
	u8 b3 = (val >> 24);

	r->data[offset + 0] = b0;
	r->data[offset + 1] = b1;
	r->data[offset + 2] = b2;
	r->data[offset + 3] = b3;
}

