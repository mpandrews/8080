#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int ana(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	// Opcodes 0xa0 through 0xa7 are ANA
	assert((opcode[0] & 0b11111000) == 0b10100000);
	uint8_t source_operand = GET_SOURCE_OPERAND(opcode[0]);

#ifdef VERBOSE
	fprintf(stderr, "ANA %c\n", get_operand_name(source_operand));
#endif

	uint8_t operand = fetch_operand_val(source_operand, cpu);
	// The Aux Carry flag is set by this operation if either operand has a
	// high-set bit 3. This must be done before assigning cpu->a.
	cpu->flags = ((1 << 3) & (cpu->a | operand)
					? cpu->flags | AUX_CARRY_FLAG
					: cpu->flags & ~AUX_CARRY_FLAG);

	cpu->a &= operand;

	/* ANA affects the carry, aux carry, zero, sign, and parity
	 * flags depending on the result. The carry flag is always reset. the
	 * Aux carry flag was already taken care of before overwriting the
	 * contents of cpu->a.
	 */
	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	// unconditionally reset the CARRY flags
	cpu->flags &= ~CARRY_FLAG;

	// Performing this operation using OPERAND MEM requires 7 cycles, and
	// it takes 4 cycles when using register operands.
	if (source_operand == OPERAND_MEM)
		cycle_wait(7);
	else
		cycle_wait(4);
	// Advance PC by one byte
	return 1;
}

int ani(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0xE6);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "ANI 0x%2.2x\n", opcode[1]);
#endif

	uint8_t operand = opcode[1];
	cpu->flags	= ((1 << 3) & (cpu->a | operand)
					     ? cpu->flags | AUX_CARRY_FLAG
					     : cpu->flags & ~AUX_CARRY_FLAG);

	// AND immediate
	cpu->a &= operand;

	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	// Clear CY and AC flags
	cpu->flags &= ~CARRY_FLAG;

	cycle_wait(7);
	return 2;
}

int xra(const uint8_t* opcode, struct cpu_state* cpu)
{
	// XRA opcode: 0b10101SSS, where SSS = source operand
	assert((opcode[0] & 0b11111000) == 0b10101000);
	uint8_t source_operand = GET_SOURCE_OPERAND(opcode[0]);

#ifdef VERBOSE
	fprintf(stderr, "XRA %c\n", get_operand_name(source_operand));
#endif

	cpu->a ^= fetch_operand_val(source_operand, cpu);

	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	// Clear CY and AC flags
	cpu->flags &= (~CARRY_FLAG & ~AUX_CARRY_FLAG);

	// If XOR memory, wait 7 cycles. If XOR register, wait 4 cycles
	get_operand_name(source_operand) == 'M' ? cycle_wait(7) : cycle_wait(4);
	return 1;
}

int xri(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0xEE);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "XRI 0x%2.2x\n", opcode[1]);
#endif

	// Exclusive OR immediate
	cpu->a ^= opcode[1];

	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	// Clear CY and AC flags
	cpu->flags &= (~CARRY_FLAG & ~AUX_CARRY_FLAG);

	cycle_wait(7);
	return 2;
}

int ora(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	// Opcodes 0xb0 through 0xb7 are ANA
	assert((opcode[0] & 0b11111000) == 0b10110000);
	uint8_t source_operand = GET_SOURCE_OPERAND(opcode[0]);

#ifdef VERBOSE
	fprintf(stderr, "ORA %c\n", get_operand_name(source_operand));
#endif

	uint8_t operand = fetch_operand_val(source_operand, cpu);
	cpu->a |= operand;

	/* ORA affects the carry, zero, sign and parity flags depending on the
	 * result. The carry and aux carry flags are always reset.
	 */
	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	// unconditionally reset the CARRY and AUX CARRY flags
	cpu->flags &= (~CARRY_FLAG & ~AUX_CARRY_FLAG);

	// getting an operand from memory takes 7 cycles, using register
	// operands takes 4 cycles and all are 1-byte instructions
	if (source_operand == OPERAND_MEM)
		cycle_wait(7);
	else
		cycle_wait(4);
	return 1;
}

int ori(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode[0] == 0b11110110);
	uint8_t operand = opcode[1];

#ifdef VERBOSE
	fprintf(stderr, "ORI 0x%2.2x\n", opcode[1]);
#endif

	// Inclusive-OR the A register with ORI's argument
	cpu->a |= operand;

	/* ORI affects the Sign, Zero, and Parity flags based on the
	 * result of the operation. The Carry and Aux Carry flags are
	 * reset unconditionally
	 */
	APPLY_ZERO_FLAG(cpu->a, cpu->flags);
	APPLY_SIGN_FLAG(cpu->a, cpu->flags);
	APPLY_PARITY_FLAG(cpu->a, cpu->flags);
	cpu->flags &= (~CARRY_FLAG & ~AUX_CARRY_FLAG);

	// ORI always takes 7 cycles and advances the PC by 2
	cycle_wait(7);
	return 2;
}

int cmp(const uint8_t* opcode, struct cpu_state* cpu)
{
	// CMP is 0xB8 - 0xBF, or 0b10111SSS
	assert((opcode[0] & 0b11111000) == 0b10111000);
	uint16_t operand =
			fetch_operand_val(GET_SOURCE_OPERAND(opcode[0]), cpu);

#ifdef VERBOSE
	fprintf(stderr,
			"CMP %c\n",
			get_operand_name(GET_SOURCE_OPERAND(opcode[0])));
#endif

	// Compare register or memory
	// Subtract content of register or memory location from the accumulator
	// The accumulator remains UNCHANGED.
	// The condition flags are set as a result of the subtraction
	// Subtraction - take two's complement and then add
	operand		= (uint8_t) ~operand;
	uint16_t result = _add(cpu->a, operand, 1, &cpu->flags);
	APPLY_CARRY_FLAG_INVERTED(result, cpu->flags);

	// If CMP memory, wait 7 cycles. If CMP register, wait 4 cycles
	get_operand_name(GET_SOURCE_OPERAND(operand)) == 'M' ? cycle_wait(7)
							     : cycle_wait(4);
	return 1;
}

int cpi(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode[0] == 0xfe);

#ifdef VERBOSE
	fprintf(stderr, "CPI 0x%2.2x\n", opcode[1]);
#endif

	/* CPI compares the next byte in memory against the accumulator.
	 * It affects all of the flags based upon the result of subtracting
	 * its argument from the accumulator. If the result is 0, the Zero
	 * flag is set. The carry flag is set if reg A < the operand. the rest
	 * of the flags are set normally based upon the result.
	 */

	// get one's complement of the operand
	uint16_t operand = opcode[1];
	operand		 = (uint8_t) ~operand;

	// get the result and set the flags, and then discard the result
	uint16_t result = _add(cpu->a, operand, 1, &cpu->flags);
	APPLY_CARRY_FLAG_INVERTED(result, cpu->flags);

	cycle_wait(7);
	return 2;
}

int rlc(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x07);
#ifdef VERBOSE
	fprintf(stderr, "RLC\n");
#endif
	(void) opcode;
	cpu->a	   = (cpu->a << 1) | (cpu->a >> 7);
	cpu->flags = (cpu->a & 1) ? cpu->flags | CARRY_FLAG
				  : cpu->flags & ~CARRY_FLAG;
	cycle_wait(4);
	return 1;
}

int rrc(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x0f);
#ifdef VERBOSE
	fprintf(stderr, "RRC\n");
#endif
	(void) opcode;
	cpu->a	   = (cpu->a >> 1) | (cpu->a << 7);
	cpu->flags = (cpu->a & 0x80) ? cpu->flags | CARRY_FLAG
				     : cpu->flags & ~CARRY_FLAG;
	cycle_wait(4);
	return 1;
}

int ral(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x17);
#ifdef VERBOSE
	fprintf(stderr, "RAL\n");
#endif
	(void) opcode;
	uint16_t shifted = (cpu->a << 1) | (cpu->flags & CARRY_FLAG);
	APPLY_CARRY_FLAG(shifted, cpu->flags);
	cpu->a = shifted;
	cycle_wait(4);
	return 1;
}

int rar(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x1f);
#ifdef VERBOSE
	fprintf(stderr, "RAR\n");
#endif
	(void) opcode;
	uint8_t outshifted_bit = cpu->a & 1;
	uint16_t shifted = (cpu->a >> 1) | ((cpu->flags & CARRY_FLAG) << 7);
	cpu->a		 = shifted;
	cpu->flags	 = outshifted_bit ? cpu->flags | CARRY_FLAG
				    : cpu->flags & ~CARRY_FLAG;
	cycle_wait(4);
	return 1;
}

int cma(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode[0] == 0x2f);
#ifdef VERBOSE
	fprintf(stderr, "CMA\n");
#endif

	// Set the A register to its complement
	cpu->a = ~cpu->a;
	cycle_wait(4);
	return 1;
}

int cmc(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode[0] == 0x3f);
#ifdef VERBOSE
	fprintf(stderr, "CMC\n");
#endif

	// XOR the flags register with the carry flag bit to toggle
	// just that one bit
	cpu->flags ^= CARRY_FLAG;
	cycle_wait(4);
	return 1;
}

int stc(const uint8_t* opcode, struct cpu_state* cpu)
{
	// Check STC opcode is 0x37
	assert(opcode[0] == 0b00110111);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "STC\n");
#endif

	// Set the carry flag
	cpu->flags |= CARRY_FLAG;

	cycle_wait(4);
	return 1;
}
