#include "cpu.h"
#include "cycle_timer.h"
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

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: POP %s\n",
			cpu->pc,
			get_register_name(opcode));
#endif

	uint16_t* rp = get_register_pair(opcode, cpu);
	*rp	     = *((uint16_t*) &cpu->memory[cpu->sp]);

	cpu->sp += 2;

	cycle_wait(10);
	return 1;
}

int xthl(uint8_t opcode, struct cpu_state* cpu)
{
	// assert that this is the correct opcode
	assert(opcode == 0b11100011);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: XTHL\n", opcode);
#endif

	// swap the contents of hl and memory[sp]
	uint16_t temp = *((uint16_t*) &cpu->memory[cpu->sp]);
	*((uint16_t*) &cpu->memory[cpu->sp]) = cpu->hl;
	cpu->hl				     = temp;

	cycle_wait(18);
	return 1;
}

int sphl(uint8_t opcode, struct cpu_state* cpu)
{
	// assert that this is the correct opcode
	assert(opcode == 0b11111001);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: SPHL\n", opcode);
#endif

	// replace sp with the contents of hl
	cpu->sp = cpu->hl;

	cycle_wait(5);
	return 1;
}

int ei(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0xFB);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: EI\n", opcode);
#endif

	// Enable interrupts
	cpu->interrupt_enable_flag = 2;

	cycle_wait(4);
	return 1;
}

int di(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0xF3);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: DI\n", opcode);
#endif

	// Disable interrupts
	cpu->interrupt_enable_flag = 0;

	cycle_wait(4);
	return 1;
}

int hlt(uint8_t opcode, struct cpu_state* cpu)
{
	// Check HLT opcode is 0x76
	assert(opcode == 0b01110110);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: HLT\n", cpu->pc);
#endif

	cpu->halt_flag = 1;
	cycle_wait(7);
	return 1;
}

int nop(uint8_t opcode, struct cpu_state* cpu)
{
	// Check NOP opcode is one of:
	// 0x00, 0x10, 0x20, 0x30, 0x08, 0x18 0x28, 0x38
	assert((opcode & 0b11000111) == 0b00000000);
	(void) opcode;
	(void) cpu;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: NOP\n", cpu->pc);
#endif

	cycle_wait(4);
	return 1;
}
