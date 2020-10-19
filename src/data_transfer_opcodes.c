#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int mov(uint8_t opcode, struct cpu_state* cpu)
{
	// Sanity check: make sure the opcode that got us here is actually
	// MOV, unless NDEBUG is defined.  A MOV instruction byte always
	// starts with 01, and all such bytes except 0x76 are MOVs.
	// 0x76, which logically should be MOV M,M is actually HALT.

	// Note that I'm using binary literals here, because it's easier
	// to cross-reference them against the manual.
	assert((opcode & 0b11000000) == 0b01000000 && opcode != 0x76);

	// Our nice verbose debug information, slow, but we don't care
	// for debug purposes.
#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: MOV %c,%c\n",
			cpu->pc,
			get_operand_name(GET_DESTINATION_OPERAND(opcode)),
			get_operand_name(GET_SOURCE_OPERAND(opcode)));
#endif
	// Fetch pointers to the operands.
	uint8_t source = fetch_operand_val(GET_SOURCE_OPERAND(opcode), cpu);
	uint8_t* dest = fetch_operand_ptr(GET_DESTINATION_OPERAND(opcode), cpu);

	*dest = source;
	// Increment the program counter, since this is a one-byte
	// instruction.

	// If either operand is memory, the instruction takes
	// an additional two cycles.
	if (GET_SOURCE_OPERAND(opcode) == OPERAND_MEM
			|| GET_DESTINATION_OPERAND(opcode) == OPERAND_MEM)
		cycle_wait(7);
	else
		cycle_wait(5);
	return 1;
}

int mvi(uint8_t opcode, struct cpu_state* cpu)
{
	// Check that we're in the right opcode.
	assert((opcode & 0b11000111) == 0b00000110);

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: MVI %c\n",
			cpu->pc,
			get_operand_name(GET_DESTINATION_OPERAND(opcode)));
#endif
	// Write the byte following the opcode to the destination.
	*fetch_operand_ptr(GET_DESTINATION_OPERAND(opcode), cpu) =
			cpu->memory[cpu->pc + 1];

	// Two-byte opcode, counting the immediate value.
	// Takes longer if writing to memory.
	if (GET_DESTINATION_OPERAND(opcode) == OPERAND_MEM)
		cycle_wait(10);
	else
		cycle_wait(7);

	return 2;
}

int lxi(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int lda(uint8_t opcode, struct cpu_state* cpu)
{
	// LDA is opcode 0x3A
	assert(opcode == 0b00111010);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: LDA\n", cpu->pc);
#endif

	// Load content of the memory location specified in the instruction
	// to register A
	uint16_t address = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	cpu->a		 = cpu->memory[address];

	cycle_wait(13);
	return 3;
}

int sta(uint8_t opcode, struct cpu_state* cpu)
{
	// STA is opcode 0x32
	assert(opcode == 0b00110010);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: STA\n", cpu->pc);
#endif

	// Store the content of the accumulator to memory location specified
	// in the instruction
	uint16_t address     = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	cpu->memory[address] = cpu->a;

	cycle_wait(13);
	return 3;
}

int lhld(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0x2a);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: LHLD\n", cpu->pc);
#endif

	// Load H and L direct
	// The content of the memory location specified by the next 2 bytes of
	// the instruction is loaded to register HL.
	uint16_t address = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	cpu->hl		 = *((uint16_t*) &cpu->memory[address]);

	cycle_wait(16);
	return 3;
}
int shld(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0x22);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: SHLD\n", cpu->pc);
#endif

	// Store H and L direct
	// The content of register HL is moved to the memory location
	// specified in the next 2 bytes of the instruction.
	uint16_t address = *((uint16_t*) &cpu->memory[cpu->pc + 1]);
	*((uint16_t*) &cpu->memory[address]) = cpu->hl;

	cycle_wait(16);
	return 3;
}

int ldax(uint8_t opcode, struct cpu_state* cpu)
{
	// LDAX opcodes are either 0x0A or 0x1A
	assert(opcode == 0b00001010 || opcode == 0b00011010);

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: LDAX %s\n",
			cpu->pc,
			get_register_pair_name_other(opcode));
#endif

	uint16_t* rp = get_register_pair_other(opcode, cpu);
	// load content of the byte at the address found at RP to register A
	cpu->a = cpu->memory[*rp];

	cycle_wait(7);
	return 1;
}

int stax(uint8_t opcode, struct cpu_state* cpu)
{
	// STAX opcodes are either 0x02 or 0x12
	assert(opcode == 0b00000010 || opcode == 0b00010010);

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: STAX %s\n",
			cpu->pc,
			get_register_pair_name_other(opcode));
#endif

	uint16_t* rp = get_register_pair_other(opcode, cpu);
	// load register A to memory at the address found in RP
	cpu->memory[*rp] = cpu->a;

	cycle_wait(7);
	return 1;
}

int xchg(uint8_t opcode, struct cpu_state* cpu)
{
	// Check XCHG opcode is 0xEB
	assert(opcode == 0b11101011);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: XCHG\n", cpu->pc);
#endif

	uint16_t temp = cpu->de;
	cpu->de	      = cpu->hl;
	cpu->hl	      = temp;

	cycle_wait(4);
	return 1;
}
