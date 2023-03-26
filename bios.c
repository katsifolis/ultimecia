#include <stdio.h>
#include <stdlib.h>

#ifndef BIOS_H
#define BIOS_H
#include "bios.h"
#endif

#ifndef TYPES_H
#define TYPES_H
#include "types.h"
#endif

BIOS*
new_bios(char* path)
{
	BIOS *b = NULL;
	long pos;

	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		perror("ERROR");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);

	pos = ftell(f);
	/* If not 512KB then exit */
	if (pos != 512*1024) {
		fprintf(stderr, "INVALID BIOS_SIZE\n");
		exit(1);
	}

	fseek(f, 0, SEEK_SET);
	
	b = malloc(sizeof(BIOS));
	b->data = malloc(sizeof(unsigned char)*pos);
	fread(b->data, 1, pos, f);

	fclose(f);

	return b;
}

u32
BIOS_load32(BIOS* b, u32 offset)
{
	return (b->data[offset + 3] << 24) | 
		   (b->data[offset + 2] << 16) | 
		   (b->data[offset + 1] <<  8) | 
		   (b->data[offset + 0]);
}
