#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

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
	fprintf(stderr, "0x%4.4x: J%s\n", cpu->pc, get_condition_name(opcode));
#endif

	// Determine which condition this version of jcond is meant to check.
	// Set conditionMet flag if the corresponding condition is.. met.
	uint8_t conditionMet = evaluate_condition(opcode, cpu->psw);

	// If the condition was met, set the program counter to the argument
	// which is the next 2 bytes after the jcond opcode. If the condition
	// was not met, then increment the program counter by 3 to the next
	// opcode
	if (conditionMet)
		cpu->pc = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	else
		cpu->pc += 3;

	return 10;
}

int call(uint8_t opcode, struct cpu_state* cpu)
{
	// The 'proper' CALL opcode is 0xcd, but 0xdd, 0xed, 0xfd are also
	// CALL opcodes, though they aren't supposed to be used
	assert((opcode & 0b11001111) == 0b11001101);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CALL\n", cpu->pc);
#endif

	// move the stack pointer
	// push the next instruction onto the stack. This is 3 bytes past the
	// current program counter to omit the CALL opcode and argument
	cpu->sp -= 2;
	*((uint16_t*) &cpu->memory[cpu->sp]) = cpu->pc + 3;

	// set the program counter to the argument supplied to by call
	cpu->pc = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	return 17;
}

int ccond(uint8_t opcode, struct cpu_state* cpu)
{
	// assert that this is the right opcode!
	assert((opcode & 0b11000111) == 0b11000100);

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: C%s\n", cpu->pc, get_condition_name(opcode));
#endif

	// Evaluate whether the given condition has been met. If the condition
	// is met, then execute the call by advancing the stack pointer,
	// pushing the address of the next opcode onto the stack, and
	// setting the program counter to point to the address of the call. If
	// the condition was not met, just advance the program counter to the
	// next opcode
	uint8_t conditionMet = evaluate_condition(opcode, cpu->psw);
	if (conditionMet)
	{
		cpu->sp -= 2;
		*((uint16_t*) &cpu->memory[cpu->sp]) = cpu->pc + 3;
		// put CALL's argument which is at cpu->pc + 1 in memory
		// into the program counter
		cpu->pc = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	}
	else
		cpu->pc += 3;

	// Executing the call if the condition is met takes 17 cycles,
	// otherwise it takes 11
	if (conditionMet)
		return 17;
	else
		return 11;
}

int ret(uint8_t opcode, struct cpu_state* cpu)
{
	// Assert that this is the correct opcode. RET should be 0xc9, but
	// 0xd9 can also be RET
	assert((opcode & 0b11101111) == 0b11001001);
	// print debugging info
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: RET\n", cpu->pc);
#endif
	// perform the ret:
	// 1. Take the contents the memory at the stack pointer and put it into
	//    the program counter.
	// 2. increment the stack pointer by 2
	cpu->pc = *((uint16_t*) &cpu->memory[cpu->sp]);
	cpu->sp += 2;

	// RET takes 10 cycles
	return 10;
}

int retcond(uint8_t opcode, struct cpu_state* cpu)
{
	// assert that this is the right opcode!
	assert((opcode & 0b11000111) == 0b11000000);

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: R%s\n", cpu->pc, get_condition_name(opcode));
#endif

	// Evaluate whether the given condition has been met. If the condition
	// is met, then execute the return by pulling the contents of memory
	// at the stack pointer into the program counter and incrementing the
	// stack pointer. If the conditon is met, just move on to the next
	// instruction.
	uint8_t conditionMet = evaluate_condition(opcode, cpu->psw);
	if (conditionMet)
	{
		cpu->pc = *((uint16_t*) &cpu->memory[cpu->sp]);
		cpu->sp += 2;
	}
	else
		cpu->pc += 1;

	// Executing ret takes 11 cycles if the condition is met,
	// otherwise it takes 5
	if (conditionMet)
		return 11;
	else
		return 5;
}

int rst(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int pchl(uint8_t opcode, struct cpu_state* cpu)
{
	// PCHL opcode should be 0xE9
	assert(opcode == 0b11101001);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: PCHL\n", cpu->pc);
#endif

	// The contents of register pair HL are copied to the PC
	cpu->pc = cpu->hl;

	return 5;
}
