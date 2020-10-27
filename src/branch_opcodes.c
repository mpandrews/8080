#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int jmp(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Check JMP opcodes are either 0xC3 or 0xCB
	assert(opcode[0] == 0b11000011 || opcode[0] == 0b11001011);
	(void) opcode;

	uint16_t memory_address = IMM16(opcode);

#ifdef VERBOSE
	fprintf(stderr, "JMP 0x%4.4x\n", memory_address);
#endif

	cpu->pc = memory_address;

	return 10;
}

int jcond(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Assert that this is the correct opcode
	// all 8 conditional jump opcodes have the two highest-order bits set,
	// and the three lowest order bits are 010.
	assert((opcode[0] & 0b11000111) == 0b11000010);

	// Verbose debug info
#ifdef VERBOSE
	fprintf(stderr,
			"J%s 0x%4.4x\n",
			get_condition_name(opcode[0]),
			IMM16(opcode));
#endif

	// Determine which condition this version of jcond is meant to check.
	// Set conditionMet flag if the corresponding condition is.. met.
	uint8_t conditionMet = evaluate_condition(opcode[0], cpu->psw);

	// If the condition was met, set the program counter to the argument
	// which is the next 2 bytes after the jcond opcode. If the condition
	// was not met, then increment the program counter by 3 to the next
	// opcode
	if (conditionMet) { cpu->pc = IMM16(opcode); }
	return 10;
}

int call(const uint8_t* opcode, struct cpu_state* cpu)
{
	// The 'proper' CALL opcode is 0xcd, but 0xdd, 0xed, 0xfd are also
	// CALL opcodes, though they aren't supposed to be used
	assert((opcode[0] & 0b11001111) == 0b11001101);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "CALL 0x%4.4x\n", IMM16(opcode));
#endif

	// move the stack pointer
	// push the next instruction onto the stack.
	cpu->sp -= 2;
	*((uint16_t*) &cpu->memory[cpu->sp]) = cpu->pc;

	// set the program counter to the argument supplied to by call
	cpu->pc = IMM16(opcode);
	return 17;
}

int ccond(const uint8_t* opcode, struct cpu_state* cpu)
{
	// assert that this is the right opcode!
	assert((opcode[0] & 0b11000111) == 0b11000100);

#ifdef VERBOSE
	fprintf(stderr,
			"C%s 0x%4.4x\n",
			get_condition_name(opcode[0]),
			IMM16(opcode));
#endif

	// Evaluate whether the given condition has been met. If the condition
	// is met, then execute the call by advancing the stack pointer,
	// pushing the address of the next opcode onto the stack, and
	// setting the program counter to point to the address of the call. If
	// the condition was not met, just advance the program counter to the
	// next opcode
	uint8_t conditionMet = evaluate_condition(opcode[0], cpu->psw);
	if (conditionMet)
	{
		cpu->sp -= 2;
		*((uint16_t*) &cpu->memory[cpu->sp]) = cpu->pc;
		// put CALL's argument which is at cpu->pc + 1 in memory
		// into the program counter
		cpu->pc = IMM16(opcode);
		return 17;
	}
	else
	{
		return 11;
	}
}

int ret(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Assert that this is the correct opcode. RET should be 0xc9, but
	// 0xd9 can also be RET
	assert((opcode[0] & 0b11101111) == 0b11001001);
	(void) opcode;
	// print debugging info
#ifdef VERBOSE
	fprintf(stderr, "RET\n");
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

int retcond(const uint8_t* opcode, struct cpu_state* cpu)
{
	// assert that this is the right opcode!
	assert((opcode[0] & 0b11000111) == 0b11000000);

#ifdef VERBOSE
	fprintf(stderr, "R%s\n", get_condition_name(opcode[0]));
#endif

	// Evaluate whether the given condition has been met. If the condition
	// is met, then execute the return by pulling the contents of memory
	// at the stack pointer into the program counter and incrementing the
	// stack pointer. If the conditon is not met, just move on to the next
	// instruction.
	uint8_t conditionMet = evaluate_condition(opcode[0], cpu->psw);
	if (conditionMet)
	{
		cpu->pc = *((uint16_t*) &cpu->memory[cpu->sp]);
		cpu->sp += 2;
		return 11;
	}
	else
	{
		return 5;
	}
}

int rst(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert((opcode[0] & 0b11000111) == 0b11000111);

#ifdef VERBOSE
	fprintf(stderr, "RST 0x%4.4x\n", opcode[0] & 0b111000);
#endif

	// Push PC onto the stack.
	cpu->sp -= 2;
	*((uint16_t*) cpu->memory + cpu->sp) = cpu->pc;
	// RST jumps to the address signified in bits 3, 4, and 5 of the opcode,
	// multiplied by eight.  As it happens, bit 3 is already the 8s place,
	// so we can just filter out all the other bits and assign that
	// directly. Neat.
	cpu->pc = opcode[0] & 0b111000;
	return 11;
}

int pchl(const uint8_t* opcode, struct cpu_state* cpu)
{
	// PCHL opcode should be 0xE9
	assert(opcode[0] == 0b11101001);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "PCHL\n");
#endif

	// The contents of register pair HL are copied to the PC
	cpu->pc = cpu->hl;
	return 5;
}
