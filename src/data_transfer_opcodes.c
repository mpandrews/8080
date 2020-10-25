#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int mov(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Sanity check: make sure the opcode that got us here is actually
	// MOV, unless NDEBUG is defined.  A MOV instruction byte always
	// starts with 01, and all such bytes except 0x76 are MOVs.
	// 0x76, which logically should be MOV M,M is actually HALT.

	// Note that I'm using binary literals here, because it's easier
	// to cross-reference them against the manual.
	assert((opcode[0] & 0b11000000) == 0b01000000 && opcode[0] != 0x76);

	// Our nice verbose debug information, slow, but we don't care
	// for debug purposes.
#ifdef VERBOSE
	fprintf(stderr,
			"MOV %c,%c\n",
			get_operand_name(GET_DESTINATION_OPERAND(opcode[0])),
			get_operand_name(GET_SOURCE_OPERAND(opcode[0])));
#endif
	// Fetch pointers to the operands.
	uint8_t source = fetch_operand_val(GET_SOURCE_OPERAND(opcode[0]), cpu);
	uint8_t* dest  = fetch_operand_ptr(
			 GET_DESTINATION_OPERAND(opcode[0]), cpu);

	*dest = source;
	// Increment the program counter, since this is a one-byte
	// instruction.

	// If either operand is memory, the instruction takes
	// an additional two cycles.
	if (GET_SOURCE_OPERAND(opcode[0]) == OPERAND_MEM
			|| GET_DESTINATION_OPERAND(opcode[0]) == OPERAND_MEM)
		cycle_wait(7);
	else
		cycle_wait(5);
	return 1;
}

int mvi(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Check that we're in the right opcode.
	assert((opcode[0] & 0b11000111) == 0b00000110);

#ifdef VERBOSE
	fprintf(stderr,
			"MVI %c 0x%2.2x\n",
			get_operand_name(GET_DESTINATION_OPERAND(opcode[0])),
			opcode[1]);
#endif
	// Write the byte following the opcode to the destination.
	*fetch_operand_ptr(GET_DESTINATION_OPERAND(opcode[0]), cpu) = opcode[1];

	// Two-byte opcode, counting the immediate value.
	// Takes longer if writing to memory.
	if (GET_DESTINATION_OPERAND(opcode[0]) == OPERAND_MEM)
		cycle_wait(10);
	else
		cycle_wait(7);

	return 2;
}

int lxi(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert((opcode[0] & 0b11001111) == 0b00000001);
#ifdef VERBOSE
	fprintf(stderr,
			"LXI %s 0x%4.4x\n",
			get_register_pair_name_other(opcode[0]),
			IMM16(opcode));
#endif

	/* LXI takes its argument (the next two bytes in memory after the
	 * opcode) and stores it in the register pair that it corresponds to.
	 * It takes 10 clock cycles and advances the program counter 3 bytes.
	 * It does not affect any condition flags.
	 */
	uint16_t* reg = get_register_pair_other(opcode[0], cpu);
	*reg	      = IMM16(opcode);
	cycle_wait(10);
	return 3;
}

int lda(const uint8_t* opcode, struct cpu_state* cpu)
{
	// LDA is opcode 0x3A
	assert(opcode[0] == 0b00111010);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "LDA 0x%4.4x\n", IMM16(opcode));
#endif

	// Load content of the memory location specified in the instruction
	// to register A
	uint16_t address = *(const uint16_t*) (opcode + 1);
	cpu->a		 = cpu->memory[address];

	cycle_wait(13);
	return 3;
}

int sta(const uint8_t* opcode, struct cpu_state* cpu)
{
	// STA is opcode 0x32
	assert(opcode[0] == 0b00110010);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "STA 0x%4.4x\n", IMM16(opcode));
#endif

	// Store the content of the accumulator to memory location specified
	// in the instruction
	uint16_t address     = *(const uint16_t*) (opcode + 1);
	cpu->memory[address] = cpu->a;

	cycle_wait(13);
	return 3;
}

int lhld(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x2a);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "LHLD 0x%4.4x\n", IMM16(opcode));
#endif

	// Load H and L direct
	// The content of the memory location specified by the next 2 bytes of
	// the instruction is loaded to register HL.
	uint16_t address = IMM16(opcode);
	cpu->hl		 = *((uint16_t*) &cpu->memory[address]);

	cycle_wait(16);
	return 3;
}
int shld(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x22);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "SHLD 0x%4.4x\n", *(const uint16_t*) (opcode + 1));
#endif

	// Store H and L direct
	// The content of register HL is moved to the memory location
	// specified in the next 2 bytes of the instruction.
	uint16_t address		     = IMM16(opcode);
	*((uint16_t*) &cpu->memory[address]) = cpu->hl;

	cycle_wait(16);
	return 3;
}

int ldax(const uint8_t* opcode, struct cpu_state* cpu)
{
	// LDAX opcodes are either 0x0A or 0x1A
	assert(opcode[0] == 0b00001010 || opcode[0] == 0b00011010);

#ifdef VERBOSE
	fprintf(stderr, "LDAX %s\n", get_register_pair_name_other(opcode[0]));
#endif

	uint16_t* rp = get_register_pair_other(opcode[0], cpu);
	// load content of the byte at the address found at RP to register A
	cpu->a = cpu->memory[*rp];

	cycle_wait(7);
	return 1;
}

int stax(const uint8_t* opcode, struct cpu_state* cpu)
{
	// STAX opcodes are either 0x02 or 0x12
	assert(opcode[0] == 0b00000010 || opcode[0] == 0b00010010);

#ifdef VERBOSE
	fprintf(stderr, "STAX %s\n", get_register_pair_name_other(opcode[0]));
#endif

	uint16_t* rp = get_register_pair_other(opcode[0], cpu);
	// load register A to memory at the address found in RP
	cpu->memory[*rp] = cpu->a;

	cycle_wait(7);
	return 1;
}

int xchg(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Check XCHG opcode is 0xEB
	assert(opcode[0] == 0b11101011);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "XCHG\n");
#endif

	uint16_t temp = cpu->de;
	cpu->de	      = cpu->hl;
	cpu->hl	      = temp;

	cycle_wait(4);
	return 1;
}
