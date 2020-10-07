#include "../include/cpu.h"
#include "../include/opcode_decls.h"
#include "../include/opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int jmp(uint8_t opcode, struct cpu_state* cpu)
{
	// Check JMP opcodes are either 0xC3 or 0xCB
	assert(opcode == 0b11000011 || opcode == 0b11001011);
	(void) opcode;

	uint16_t memory_address = *((uint16_t*) &cpu->memory[cpu->pc + 1]);

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: JMP 0x%4.4x\n", cpu->pc, memory_address);
#endif

	cpu->pc = memory_address;

#ifdef VERBOSE
	print_registers(cpu);
#endif

	return 10;
}

int jcond(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int call(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ccond(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ret(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int retcond(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int rst(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int pchl(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}
