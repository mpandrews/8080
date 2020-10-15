#include "cpu.h"

#include <assert.h>
#include <stdio.h>

// IN
int hw_in(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: IN (Hardware: none)\n", cpu->pc);
#endif
	++cpu->pc;
	return 10;
}

// OUT
int hw_out(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11010011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: OUT (Hardware: none)\n", cpu->pc);
#endif
	++cpu->pc;
	return 10;
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
