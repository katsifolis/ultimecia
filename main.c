#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "types.h"
#include "cpu.h"
#include "interconnect.h"
#include "bios.h"
#include "mem.h"


int
main(int argc, char **argv)
{
	CPU *cpu;
	Interconnect *inter;
	BIOS *b;
	RAM *r;

	if (argc < 2) {
		fprintf(stderr, "No rom was given\n");
		exit(EXIT_FAILURE);
	}

	r = new_ram();
	b = new_bios(argv[1]);
	inter = new_interconnect(b, r);
	cpu = new_cpu(inter);

	while(1) {
		CPU_run_next_instruction(cpu);
	}

	return 0;
}
