#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>

int add_adc(uint8_t opcode, struct cpu_state* cpu)
{
	assert((opcode & 0b11110000) == 0b10000000);
#ifdef VERBOSE
	// This looks funky, but the idea is: if bit 3 of the opcode
	// is set, then this is ADC.  If it's not, this is ADD.
	// So we add the reverse of that bit to the character C, and if we have
	// add, we end up with D.
	fprintf(stderr,
			"0x%4.4x: AD%c %c\n",
			cpu->pc,
			!(opcode & (1 << 3)) + 'C',
			get_operand_name(GET_SOURCE_OPERAND(opcode)));
#endif
	uint8_t operand = fetch_operand_val(GET_SOURCE_OPERAND(opcode), cpu);
	if (opcode & (1 << 3) && cpu->flags & CARRY_FLAG) ++operand;

	uint16_t result = _add(cpu->a, operand, &cpu->flags);
	APPLY_CARRY_FLAG(result, cpu->flags);
	cpu->a = result;
	++cpu->pc;
	return 4;
}

int adi(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int adc(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int aci(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int sub_sbb(uint8_t opcode, struct cpu_state* cpu)
{
	assert((opcode & 0b11110000) == 0b10010000);
#ifdef VERBOSE
	fprintf(stderr,
			"0x%4.4x: S%cB %c\n",
			cpu->pc,
			(opcode & 0b00001000 ? 'B' : 'U'),
			get_operand_name(GET_SOURCE_OPERAND(opcode)));
#endif
	uint16_t operand = fetch_operand_val(GET_SOURCE_OPERAND(opcode), cpu);
	// Per the manual, the carry flag is applied (if appropriate)
	// prior to taking the two's complement.
	if (opcode & (1 << 3) && cpu->flags & CARRY_FLAG) ++operand;
	operand = (uint8_t) ~operand;
	/* Find two's complement.  This is a little hairy: first we need
	 * to invert the bits, and cast the inverted version to char length.
	 * The reason is that without the cast, we'd get all the high bits,
	 * which will be flipped to 1s, and we don't want that.
	 * If we cast and then invert, we end up with an all-1 high byte,
	 * since I suppose the compiler is doing everything in a short,
	 * meaning that we'd cast away the high byte, store the result in an
	 * unsigned short, and _then_ invert.
	 * So we need to invert and cast, in that specific order.
	 *
	 * We can't just do all of this in a char-sized package to begin with
	 * because we need to be able to roll over into the ninth bit
	 * when we do the +1, otherwise 0 becomes -1, which is ungood.
	 */
	++operand;
	// Add carry flag if this is SBB.
	// Add exactly as we would if this were an addition.
	uint16_t result = _add(cpu->a, operand, &cpu->flags);
	APPLY_CARRY_FLAG_INVERTED(result, cpu->flags);
	cpu->a = result;
	++cpu->pc;
	return 4;
}

int sui(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int sbb(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int sbi(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int inr(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int dcr(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int inx(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int dcx(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int dad(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}

int daa(uint8_t opcode, struct cpu_state* cpu)
{
	// TODO
	return placeholder(opcode, cpu);
}
