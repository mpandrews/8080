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

	switch (GET_REGISTER_PAIR(opcode))
	{
	case REGISTER_PAIR_BC:
#ifdef VERBOSE
		fprintf(stderr, "0x%4.4x: POP B\n", cpu->pc);
#endif
		cpu->bc = *((uint16_t*) &cpu->memory[cpu->sp]);
		break;
	case REGISTER_PAIR_DE:
#ifdef VERBOSE
		fprintf(stderr, "0x%4.4x: POP D\n", cpu->pc);
#endif
		cpu->de = *((uint16_t*) &cpu->memory[cpu->sp]);
		break;
	case REGISTER_PAIR_HL:
#ifdef VERBOSE
		fprintf(stderr, "0x%4.4x: POP H\n", cpu->pc);
#endif
		cpu->hl = *((uint16_t*) &cpu->memory[cpu->sp]);
		break;
	case REGISTER_PAIR_SP_PSW:
#ifdef VERBOSE
		fprintf(stderr, "0x%4.4x: POP PSW\n", cpu->pc);
#endif
		cpu->psw = *((uint16_t*) &cpu->memory[cpu->sp]);
		break;
	default:
#ifdef VERBOSE
		fprintf(stderr, "ERROR: register pair not found!");
#endif
		exit(1);
	}

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
	assert((opcode == 0b00000000) | (opcode == 0b00010000)
			| (opcode == 0b00100000) | (opcode == 0b00110000)
			| (opcode == 0b00001000) | (opcode == 0b00011000)
			| (opcode == 0b00101000) | (opcode == 0b00111000));
	(void) opcode;

	cpu->pc++;
	return 4;
}
