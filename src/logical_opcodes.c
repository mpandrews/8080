#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int ana(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ani(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int xra(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int xri(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ora(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ori(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cmp(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cpi(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int rlc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int rrc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int ral(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int rar(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cma(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0x2f);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CMA\n", cpu->pc);
#endif

	// XOR the accumulator register to complement each bit
	cpu->a ^= 0xff;
	return 1;
}

int cmc(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0x3f);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CMC\n", cpu->pc);
#endif

	// XOR the flags register with the carry flag bit to toggle
	// just that one bit
	cpu->flags ^= CARRY_FLAG;
	return 1;
}

int stc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}
