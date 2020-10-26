#include "cpu.h"

#include <assert.h>
#include <stdio.h>

extern void cycle_wait(int);

// IN
int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "IN 0x%2.2x (Hardware: cpudiag)\n", opcode[1]);
#endif
	return 10;
}

// OUT
int hw_out(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11010011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "OUT 0x%2.2x (Hardware: cpudiag)\n", opcode[1]);
#endif
	// Thanks to emulator101.com for figuring out how the ROM tries to
	// print.
	if (cpu->c == 9)
	{
		uint8_t* s = cpu->memory + cpu->de;
		while (s < cpu->memory + MAX_MEMORY - 1 && *s != '$')
			printf("%c", *s++);
	}
	else
		printf("%c", cpu->e);

	fflush(NULL);
	return 10;
}

// Interrupt Hook
int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*))
{
	return op_func(opcode, cpu);
}

// Init Struct
void* hw_init_struct() { return NULL; }

// Destroy Struct
void hw_destroy_struct(void* hw_struct)
{
	(void) hw_struct;
	return;
}
