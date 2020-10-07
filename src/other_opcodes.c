#include "../include/cpu.h"
#include "../include/opcode_helpers.h"

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
	// TODO
	return placeholder(opcode, cpu);
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
	// TODO
	return placeholder(opcode, cpu);
}
