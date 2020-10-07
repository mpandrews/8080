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
	// Assert that this is the correct opcode
	// all 8 conditional jump opcodes have the two highest-order bits set,
	// and the three lowest order bits are 010.
	assert((opcode & 0b11000111) == 0b11000010);

	// Verbose debug info
#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: J%s\n",
			cpu->pc,
			get_condition_name(opcode));
#endif

	// Determine which condition this version of jcond is meant to check.
	// Set conditionMet flag if the corresponding condition is.. met.
	uint8_t conditionMet = evaluate_condition(opcode, cpu->psw);

	// If the condition was met, set the program counter to the argument
	// which is the next 2 bytes after the jcond opcode. If the condition
	// was not met, then increment the program counter by 3 to the next
	// opcode
	if(conditionMet)
		cpu->pc = *((uint16_t*) &cpu->memory[cpu->pc+1]);
	else
		cpu->pc += 3;

	// More debug information.
#ifdef VERBOSE
	print_registers(cpu);
#endif

	return 10;
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
