#ifndef OPCODES
#define OPCODES

#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Certain opcodes embed a source, destination, or both directly into
 * the opcode byte.  When this is done, the pattern is always the same:
 * ??DDDSSS, where DDD is the three-bit code for the destination, and SSS
 * is the three-bit code for the source. Even when only one or the other is
 * present, the three bits are always in the same spot.  This was a really
 * clever and thoughtful technique on Intel's part.
 *
 * So, for example, MOV is:
 * 	01DDDSSS
 * And MVI is:
 * 	00DDD110
 *
 * In the manual, the M versions are often listed separately, but in fact
 * they follow the same pattern, with the 110 code for M being used.
 */

#define GET_SOURCE_OPERAND(byte)      (byte & 0x7)
#define GET_DESTINATION_OPERAND(byte) ((byte >> 3) & 0x7)

// Define to extract the register pair information from certain
// opcodes, like LDAX.
#define GET_REGISTER_PAIR(byte) ((byte >> 4) & 0x3)

// Define to extract the flag an opcode wants to operate on.
#define GET_CONDITION(byte) ((byte >> 3) & 0x7)

/* These defines cover the actual three bit codes used in DDD and SSS fields.
 */
#define OPERAND_REG_B (0b000)
#define OPERAND_REG_C (0b001)
#define OPERAND_REG_D (0b010)
#define OPERAND_REG_E (0b011)
#define OPERAND_REG_H (0b100)
#define OPERAND_REG_L (0b101)
#define OPERAND_MEM   (0b110)
#define OPERAND_REG_A (0b111)

// The two bit codes used in RP fields.
#define REGISTER_PAIR_BC     (0b00)
#define REGISTER_PAIR_DE     (0b01)
#define REGISTER_PAIR_HL     (0b10)
#define REGISTER_PAIR_SP_PSW (0b11)

#define CONDITION_NOT_ZERO    (0b000)
#define CONDITION_ZERO	      (0b001)
#define CONDITION_NO_CARRY    (0b010)
#define CONDITION_CARRY	      (0b011)
#define CONDITION_PARITY_ODD  (0b100)
#define CONDITION_PARITY_EVEN (0b101)
#define CONDITION_SIGN	      (0b110)
#define CONDITION_NO_SIGN     (0b111)

// Returns 1 if even parity, 0 if odd parity.
static inline uint8_t check_parity(uint8_t value)
{
	value ^= value >> 4;
	value ^= value >> 2;
	value ^= value >> 1;
	return ~value & 1;
}

// Check to see if a result is zero, set the flag as appopriate.
#define APPLY_ZERO_FLAG(value, flags) \
	(flags = (!value ? flags | ZERO_FLAG : flags & ~ZERO_FLAG))
// Check to see if a result is negative: just inspect the highest bit.
#define APPLY_SIGN_FLAG(value, flags) \
	(flags = (value & (1 << 7) ? flags | SIGN_FLAG : flags & ~SIGN_FLAG))
// Because we use a 16-bit variable to hold our results, we can just
// check to see if any bits in the second byte are set to check for a carry.
#define APPLY_CARRY_FLAG(value, flags) \
	(flags = (value & (0xff00) ? flags | CARRY_FLAG : flags & ~CARRY_FLAG))
/* Setting the aux carry flag is gnarly, unfortunately.
 * So here's what's happening.  We want to measure whether there was a carry
 * OUT of bit 3 (0 indexed), which is the same as checking if there was a
 * carry INTO bit 4.
 * If bit 4 of the two operands are the same, bit 4 of the result should be
 * 0.  If they're different, bit 4 should be 1.  That is to say, if both
 * operands have 0 in bit 4, then obviously that should be 0 in the result.
 * If they both have 1, then we should have 0 because we carried into bit 5.
 * So, we XOR those two bits in the operands together and get our expected
 * result bit.  We can then XOR it again with the real result bit: if it's true,
 * then we didn't get what we expected, which means there was a carry into 4,
 * so we set the flag.  We simplify it a little by just XORing the three
 * operands together and then clearing everything except bit 4.
 */
#define APPLY_AUX_CARRY_FLAG(op1, op2, result, flags)                 \
	(flags = (op1 ^ op2 ^ result) & 0x10 ? flags | AUX_CARRY_FLAG \
					     : flags & ~AUX_CARRY_FLAG)

// To apply the parity flag, we just call the parity-checker function above.
#define APPLY_PARITY_FLAG(value, flags)                     \
	(flags = (check_parity(value) ? flags | PARITY_FLAG \
				      : flags & ~PARITY_FLAG))

/* fetch_operand() is a helper function, which takes as its first argument
 * a copy of JUST the three bits representing a source or destination
 * code, shifted right in the case of a source code. Its second
 * argument is a pointer to the CPU state.
 *
 * It returns a pointer to the memory address of the desired operand.
 *
 * The intended use is as follows:
 *
 * operand_pointer = fetch_operand(GET_SOURCE_OPERAND(opcode), cpu);
 */

static inline uint8_t* fetch_operand(
		uint8_t operand_field, const struct cpu_state* cpu)
{
	switch (operand_field)
	{
	case OPERAND_REG_B: return &HIGH_REG8(cpu->bc);
	case OPERAND_REG_C: return &LOW_REG8(cpu->bc);
	case OPERAND_REG_D: return &HIGH_REG8(cpu->de);
	case OPERAND_REG_E: return &LOW_REG8(cpu->de);
	case OPERAND_REG_H: return &HIGH_REG8(cpu->hl);
	case OPERAND_REG_L: return &LOW_REG8(cpu->hl);
	case OPERAND_MEM: return cpu->memory + cpu->hl;
	case OPERAND_REG_A: return &HIGH_REG8(cpu->psw);
	default:
		fprintf(stderr,
				"ERROR: fetch_operand()"
				"was passed 0x%2.2x, which is"
				"out of range!",
				operand_field);
		exit(1);
	}
}

static inline char get_operand_name(uint8_t operand_field)
{
	switch (operand_field)
	{
	case OPERAND_REG_B: return 'B';
	case OPERAND_REG_C: return 'C';
	case OPERAND_REG_D: return 'D';
	case OPERAND_REG_E: return 'E';
	case OPERAND_REG_H: return 'H';
	case OPERAND_REG_L: return 'L';
	case OPERAND_MEM: return 'M';
	case OPERAND_REG_A: return 'A';
	default: return '?';
	}
}

static inline void print_registers(const struct cpu_state* cpu)
{
	uint8_t flags[9];
	uint8_t* f = flags;
	for (uint8_t mask = 0x80; mask; mask >>= 1, ++f)
		*f = mask & LOW_REG8(cpu->psw) ? '1' : '0';
	*f = 0;
	fprintf(stderr,
			"\tPC:  0x%4.4x -> 0x%2.2x\n"
			"\tBC:  0x%4.4x -> 0x%2.2x\n"
			"\tDE:  0x%4.4x -> 0x%2.2x\n"
			"\tHL:  0x%4.4x -> 0x%2.2x\n"
			"\tPSW: 0x%4.4x\n"
			"\tSP:  0x%4.4x -> 0x%2.2x\n"
			"\tAddress Bus: 0x%4.4x\n"
			"\tData Bus: 0x%2.2x\n"
			"\tFlags: %s\n"
			"\t       SZ-A-P-C\n",
			cpu->pc,
			cpu->memory[cpu->pc],
			cpu->bc,
			cpu->memory[cpu->bc],
			cpu->de,
			cpu->memory[cpu->de],
			cpu->hl,
			cpu->memory[cpu->hl],
			cpu->psw,
			cpu->sp,
			cpu->memory[cpu->sp],
			*cpu->address_bus,
			*cpu->data_bus,
			flags);
}

#endif
