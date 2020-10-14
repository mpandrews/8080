#include "cpu.h"
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

	// Advance PC by one byte
	++cpu->pc;
	// Performing this operation using OPERAND MEM requires 7 cycles, and
	// it takes 4 cycles when using register operands.
	if (source_operand == OPERAND_MEM)
		return 7;
	else
		return 4;
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
	++cpu->pc;
	if (source_operand == OPERAND_MEM)
		return 7;
	else
		return 4;
}

int ori(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cmp(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int cpi(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
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
	// TODO
	return placeholder(opcode, cpu);
}

int cmc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int stc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}
