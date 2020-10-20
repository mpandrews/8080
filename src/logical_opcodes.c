#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int ana(uint8_t opcode, struct cpu_state* cpu)
{
	(void) opcode;
	// Opcodes 0xa0 through 0xa7 are ANA
	assert((opcode & 0b11111000) == 0b10100000);
	uint8_t source_operand = GET_SOURCE_OPERAND(opcode);

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: ANA %c\n",
			cpu->pc,
			get_operand_name(source_operand));
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
	(void) opcode;
	// Opcodes 0xb0 through 0xb7 are ANA
	assert((opcode & 0b11111000) == 0b10110000);
	uint8_t source_operand = GET_SOURCE_OPERAND(opcode);

#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: ORA %c\n",
			cpu->pc,
			get_operand_name(source_operand));
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

int ori(uint8_t opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode == 0b11110110);
	uint8_t operand = cpu->memory[cpu->pc + 1];

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: ORI\n", cpu->pc);
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

int cmp(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cpi(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0xfe);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CPI\n", cpu->pc);
#endif

	/* CPI compares the next byte in memory against the accumulator.
	 * It affects all of the flags based upon the result of subtracting
	 * its argument from the accumulator. If the result is 0, the Zero
	 * flag is set. The carry flag is set if reg A < the operand. the rest
	 * of the flags are set normally based upon the result.
	 */
	// get two's complement of the operand
	uint16_t operand = ~(cpu->memory[cpu->sp]);
	operand++;
	fprintf(stderr, "%d\n", operand);

	//figure out how to set the aux carry flag later
	
	// add the contents of a
	operand += cpu->a;

	// set flags
	APPLY_CARRY_FLAG_INVERTED(operand, cpu->flags);
	APPLY_ZERO_FLAG(operand, cpu->flags);
	APPLY_SIGN_FLAG(operand, cpu->flags);
	APPLY_PARITY_FLAG(operand, cpu->flags);

	cycle_wait(7);
	return 2;
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
	(void) opcode;
	assert(opcode == 0x2f);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CMA\n", cpu->pc);
#endif

	// Set the A register to its complement
	cpu->a = ~cpu->a;
	cycle_wait(4);
	return 1;
}

int cmc(uint8_t opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert(opcode == 0x3f);
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: CMC\n", cpu->pc);
#endif

	// XOR the flags register with the carry flag bit to toggle
	// just that one bit
	cpu->flags ^= CARRY_FLAG;
	cycle_wait(4);
	return 1;
}

int stc(uint8_t opcode, struct cpu_state* cpu)
{
	// Check STC opcode is 0x37
	assert(opcode == 0b00110111);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: STC\n", cpu->pc);
#endif

	// Set the carry flag
	cpu->flags |= CARRY_FLAG;

	cycle_wait(4);
	return 1;
}
