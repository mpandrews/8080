#include "cpu.h"

#include <assert.h>
#include <stdio.h>

extern void cycle_wait(int);

// IN
int hw_in(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: IN (Hardware: cpudiag)\n", cpu->pc);
#endif
	cycle_wait(10);
	return 2;
}

// OUT
int hw_out(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11010011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: OUT (Hardware: cpudiag)\n", cpu->pc);
#endif
	// Thanks to emulator101.com for figuring out how the ROM tries to
	// print.
	if (cpu->c == 9)
	{
		uint8_t* s = cpu->memory + cpu->de;
		while (s < cpu->memory + MAX_MEMORY - 1 && *s != '$'
				&& *s <= 0x7f)
			printf("%c", *s++);
	}
	else
		printf("%c", cpu->e);

	cycle_wait(10);
	return 2;
}

// Interrupt Hook
void hw_interrupt_hook(uint8_t opcode, struct cpu_state* cpu)
{
	(void) opcode;
	(void) cpu;
	return;
}

// Init Struct
void* hw_init_struct() { return NULL; }

// Destroy Struct
void hw_destroy_struct(void* hw_struct)
{
	(void) hw_struct;
	return;
}
