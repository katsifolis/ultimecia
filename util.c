#include "util.h"
#include "types.h"

u8
UTIL_contains(u8 range, u32 addr)
{

	switch (range) {
	case BIOS_RANGE:
		if (addr >= BIOS_START && addr < BIOS_START + BIOS_SIZE)
			return TRUE;
		break;
	case MEMCONTROL_RANGE:
		if (addr >= MEMCONTROL_START && addr < MEMCONTROL_START + MEMCONTROL_SIZE)
			return TRUE;
		break;
	case RAM_SIZE_RANGE:
		if (addr >= RAM_SIZE_START && addr < RAM_SIZE_START + RAM_SIZE_SIZE)
			return TRUE;
		break;
	case CACHECONTROL_RANGE:
		if (addr >= CACHECONTROL_START && addr < CACHECONTROL_START + CACHECONTROL_SIZE)
			return TRUE;
		break;
	case RAM_RANGE:
		if (addr >= RAM_START && addr < RAM_START + RAM_SIZE)
			return TRUE;
		break;
	default:
		break;
	};

	return FALSE;

}
