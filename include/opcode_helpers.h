#ifndef OPCODES
#define OPCODES

#include "cpu.h"

#include <assert.h>
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
#define CONDITION_NO_SIGN     (0b110)
#define CONDITION_SIGN	      (0b111)

#define IMM16(opcode_ptr) (*(const uint16_t*) (opcode_ptr + 1))

// Returns 1 if even parity, 0 if odd parity.
__attribute__((const)) static inline uint8_t check_parity(uint8_t value)
{
	value ^= value >> 4;
	value ^= value >> 2;
	value ^= value >> 1;
	return ~value & 1;
}

// Check to see if a result is zero, set the flag as appropriate.
#define APPLY_ZERO_FLAG(value, flags) \
	(flags = (!value ? flags | ZERO_FLAG : flags & ~ZERO_FLAG))
// Check to see if a result is negative: just inspect the highest bit.
#define APPLY_SIGN_FLAG(value, flags) \
	(flags = (value & (1 << 7) ? flags | SIGN_FLAG : flags & ~SIGN_FLAG))
// Because we use a 16-bit variable to hold our results, we can just
// check to see if the least sig bit in the second byte is set to check for a
// carry.
#define APPLY_CARRY_FLAG(value, flags) \
	(flags = (value & (0x0100) ? flags | CARRY_FLAG : flags & ~CARRY_FLAG))
// Subtraction operations use reverse the logic for setting carry.
#define APPLY_CARRY_FLAG_INVERTED(value, flags) \
	(flags = (value & (0x0100) ? flags & ~CARRY_FLAG : flags | CARRY_FLAG))

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

__attribute__((pure)) static inline uint8_t* fetch_operand_ptr(
		uint8_t operand_field, struct cpu_state* cpu)
{
	switch (operand_field)
	{
	case OPERAND_REG_B: return &cpu->b;
	case OPERAND_REG_C: return &cpu->c;
	case OPERAND_REG_D: return &cpu->d;
	case OPERAND_REG_E: return &cpu->e;
	case OPERAND_REG_H: return &cpu->h;
	case OPERAND_REG_L: return &cpu->l;
	case OPERAND_MEM: return cpu->memory + cpu->hl;
	case OPERAND_REG_A: return &cpu->a;
	default:
		fprintf(stderr,
				"ERROR: fetch_operand_ptr()"
				"was passed 0x%2.2x, which is"
				"out of range!",
				operand_field);
		exit(1);
	}
}

__attribute__((pure)) static inline uint8_t fetch_operand_val(
		uint8_t operand_field, struct cpu_state const* cpu)
{
	switch (operand_field)
	{
	case OPERAND_REG_B: return cpu->b;
	case OPERAND_REG_C: return cpu->c;
	case OPERAND_REG_D: return cpu->d;
	case OPERAND_REG_E: return cpu->e;
	case OPERAND_REG_H: return cpu->h;
	case OPERAND_REG_L: return cpu->l;
	case OPERAND_MEM: return cpu->memory[cpu->hl];
	case OPERAND_REG_A: return cpu->a;
	default:
		fprintf(stderr,
				"ERROR: fetch_operand_val()"
				"was passed 0x%2.2x, which is"
				"out of range!",
				operand_field);
		exit(1);
	}
}

__attribute__((const)) static inline char get_operand_name(
		uint8_t operand_field)
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

/* get_condition_name returns the name of the conditional opcode based
 * on bits 3-5 of the opcode.
 * NZ - not zero
 * Z  - zero
 * NC - no carry
 * C  - carry
 * PO - parity odd
 * PE - parity even
 * M  - "minus", sign bit is set
 * P - "plus", sign bit is reset
 */
__attribute__((const)) static inline char* get_condition_name(
		const uint8_t opcode)
{
	switch (GET_CONDITION(opcode))
	{
	case CONDITION_NOT_ZERO: return "NZ";
	case CONDITION_ZERO: return "Z";
	case CONDITION_NO_CARRY: return "NC";
	case CONDITION_CARRY: return "C";
	case CONDITION_PARITY_ODD: return "PO";
	case CONDITION_PARITY_EVEN: return "PE";
	case CONDITION_SIGN: return "M";
	case CONDITION_NO_SIGN: return "P";
	}
	return "??";
}

/* evaluate_condition is a helper function that takes an opcode and the contents
 * of the flags register as arguments. It determines which condition the opcode
 * indicates, switches on the condition, and returns the results of the
 * evaluated condition*/
__attribute__((const)) static inline uint8_t evaluate_condition(
		const uint8_t opcode, const uint16_t psw)
{
	switch (GET_CONDITION(opcode))
	{
	case CONDITION_NOT_ZERO: return !(psw & ZERO_FLAG);
	case CONDITION_ZERO: return psw & ZERO_FLAG;
	case CONDITION_NO_CARRY: return !(psw & CARRY_FLAG);
	case CONDITION_CARRY: return psw & CARRY_FLAG;
	// Parity bit is set when parity is even
	case CONDITION_PARITY_ODD: return !(psw & PARITY_FLAG);
	// Parity bit is set when parity is even
	case CONDITION_PARITY_EVEN: return psw & PARITY_FLAG;
	case CONDITION_SIGN: return psw & SIGN_FLAG;
	case CONDITION_NO_SIGN: return !(psw & SIGN_FLAG);
	default:
		fprintf(stderr,
				"ERROR: evaluate_condition()"
				"was passed 0x%4.4x, which did"
				"not evaluate to any condition!",
				opcode);
		exit(1);
	}
	return 0;
}

__attribute__((pure)) static inline uint16_t* get_register_pair_pushpop(
		const uint8_t opcode, struct cpu_state* cpu)
{
	switch (GET_REGISTER_PAIR(opcode))
	{
	case REGISTER_PAIR_BC: return &cpu->bc;
	case REGISTER_PAIR_DE: return &cpu->de;
	case REGISTER_PAIR_HL: return &cpu->hl;
	case REGISTER_PAIR_SP_PSW: return &cpu->psw;
	default: exit(1);
	}
}

__attribute__((const)) static inline const char* get_register_pair_name_pushpop(
		const uint8_t opcode)
{
	switch (GET_REGISTER_PAIR(opcode))
	{
	case REGISTER_PAIR_BC: return "B";
	case REGISTER_PAIR_DE: return "D";
	case REGISTER_PAIR_HL: return "H";
	case REGISTER_PAIR_SP_PSW: return "PSW";
	default: exit(1);
	}
}

__attribute__((pure)) static inline uint16_t* get_register_pair_other(
		const uint8_t opcode, struct cpu_state* cpu)
{
	switch (GET_REGISTER_PAIR(opcode))
	{
	case REGISTER_PAIR_BC: return &cpu->bc;
	case REGISTER_PAIR_DE: return &cpu->de;
	case REGISTER_PAIR_HL: return &cpu->hl;
	case REGISTER_PAIR_SP_PSW: return &cpu->sp;
	default: exit(1);
	}
}

__attribute__((const)) static inline const char* get_register_pair_name_other(
		const uint8_t opcode)
{
	switch (GET_REGISTER_PAIR(opcode))
	{
	case REGISTER_PAIR_BC: return "B";
	case REGISTER_PAIR_DE: return "D";
	case REGISTER_PAIR_HL: return "H";
	case REGISTER_PAIR_SP_PSW: return "SP";
	default: exit(1);
	}
}

static inline uint16_t _add(
		uint16_t left, uint16_t right, uint8_t carry, uint8_t* flags)
{
	uint16_t result = left + right + carry;
	APPLY_AUX_CARRY_FLAG(left, right, result, *flags);
	APPLY_ZERO_FLAG((uint8_t) result, *flags);
	APPLY_PARITY_FLAG(result, *flags);
	APPLY_SIGN_FLAG(result, *flags);
	return result;
}

static inline void write8(struct cpu_state* cpu, uint16_t offset, uint8_t value)
{
	if (!cpu->rom_mask[offset >> cpu->mask_shift])
		cpu->memory[offset] = value;
#ifdef VERBOSE
	else
		fprintf(stderr,
				"Attempted write to read-only program memory "
				"at 0x%4.4x!\n",
				offset);
#endif
}

static inline void write16(
		struct cpu_state* cpu, uint16_t offset, uint16_t value)
{
	write8(cpu, offset, (uint8_t) value);
	write8(cpu, offset + 1, *(((uint8_t*) &value) + 1));
}

#endif
