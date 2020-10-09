#include "cpu.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int placeholder(uint8_t opcode, struct cpu_state* cpu)
{
	(void) cpu;
	fprintf(stderr,
			"Hi!  You've reached opcode 0x%2.2x!\n"
			"We're not here to take your call right now.\n"
			"Please implement me, or leave a message after the "
			"tone.\n",
			opcode);
	exit(1);
}

int push(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int pop(uint8_t opcode, struct cpu_state* cpu)
{
	// Check POP opcode is one of: 0xC1, 0xD1, 0xE1, 0xF1
	// POP opcode should look like: 11RP0001, where RP is a register pair
	assert((opcode & 0b11001111) == 0b11000001);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: POP %s\n",
			cpu->pc,
			get_register_name(opcode));
#endif

	uint16_t* rp = get_register_pair(opcode, cpu);
	*rp	     = *((uint16_t*) &cpu->memory[cpu->sp]);

	cpu->sp += 2;
	cpu->pc++;

	return 10;
}

int xthl(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int sphl(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int in(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int out(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ei(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int di(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int hlt(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int nop(uint8_t opcode, struct cpu_state* cpu)
{
	// Check NOP opcode is one of:
	// 0x00, 0x10, 0x20, 0x30, 0x08, 0x18 0x28, 0x38
	assert((opcode & 0b11000111) == 0b00000000);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: NOP\n", cpu->pc);
#endif

	cpu->pc++;
	return 4;
}
