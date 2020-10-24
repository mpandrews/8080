#include "cpu.h"
#include "cycle_timer.h"
#include "opcode_decls.h"
#include "opcode_helpers.h"

#include <assert.h>

int add_adc(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert((opcode[0] & 0b11110000) == 0b10000000);
#ifdef VERBOSE
	// This looks funky, but the idea is: if bit 3 of the opcode
	// is set, then this is ADC.  If it's not, this is ADD.
	// So we add the reverse of that bit to the character C, and if we have
	// add, we end up with D.
	fprintf(stderr,
			"AD%c %c\n",
			!(opcode[0] & (1 << 3)) + 'C',
			get_operand_name(GET_SOURCE_OPERAND(opcode[0])));
#endif
	uint16_t operand =
			fetch_operand_val(GET_SOURCE_OPERAND(opcode[0]), cpu);
	// if (opcode & (1 << 3) && cpu->flags & CARRY_FLAG) ++operand;
	uint16_t result = _add(cpu->a,
			operand,
			(opcode[0] & (1 << 3) && cpu->flags & CARRY_FLAG),
			&cpu->flags);
	APPLY_CARRY_FLAG(result, cpu->flags);
	cpu->a = result;
	cycle_wait(4);
	return 1;
}

int adi(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0xc6);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "ADI 0x%2.2x\n", opcode[1]);
#endif

	// Add immediate
	// The content of the second byte of the instruction is added to
	// the content of the accumulator.
	uint16_t operand = opcode[1];
	uint16_t result	 = _add(cpu->a, operand, 0, &cpu->flags);
	APPLY_CARRY_FLAG(result, cpu->flags);
	cpu->a = result;

	cycle_wait(7);
	return 2;
}

int aci(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0xce);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "ACI 0x%2.2x\n", opcode[1]);
#endif

	// Add immediate with carry
	// The content of the second byte of the instruction and the
	// content of the carry flag are added to the contents of the
	// accumulator.
	uint16_t operand = opcode[1];
	uint16_t result	 = _add(
			 cpu->a, operand, cpu->flags & CARRY_FLAG, &cpu->flags);
	APPLY_CARRY_FLAG(result, cpu->flags);
	cpu->a = result;

	cycle_wait(7);
	return 2;
}

int sub_sbb(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert((opcode[0] & 0b11110000) == 0b10010000);
#ifdef VERBOSE
	fprintf(stderr,
			"S%cB %c\n",
			(opcode[0] & 0b00001000 ? 'B' : 'U'),
			get_operand_name(GET_SOURCE_OPERAND(opcode[0])));
#endif
	uint16_t operand =
			fetch_operand_val(GET_SOURCE_OPERAND(opcode[0]), cpu);
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
	operand		= (uint8_t) ~operand;
	uint16_t result = _add(cpu->a,
			operand,
			!(opcode[0] & (1 << 3) && cpu->flags & CARRY_FLAG),
			&cpu->flags);
	APPLY_CARRY_FLAG_INVERTED(result, cpu->flags);
	cpu->a = result;

	cycle_wait(4);
	return 1;
}

int sui_sbi(const uint8_t* opcode, struct cpu_state* cpu)
{
	// SUI is 0xd6 and SBI is 0xde
	assert((opcode[0] & 0b11110111) == 0b11010110);
#ifdef VERBOSE
	fprintf(stderr,
			"S%cI 0x%2.2x\n",
			(opcode[0] & 0b00001000 ? 'B' : 'U'),
			opcode[1]);
#endif
	uint16_t operand = opcode[1];
	/* Find two's complement.  See sub_sbb() function comments for a
	 * detailed description about the nit-picky and many-faceted details
	 * of this process in C that result in the following two lines of code.
	 */
	operand		= (uint8_t) ~operand;
	uint16_t result = _add(cpu->a,
			operand,
			!(opcode[0] & (1 << 3) && cpu->flags & CARRY_FLAG),
			&cpu->flags);
	APPLY_CARRY_FLAG_INVERTED(result, cpu->flags);

	// apply the lower 8-bits of result to register A
	cpu->a = result;

	cycle_wait(7);
	return 2;
}

int inr(const uint8_t* opcode, struct cpu_state* cpu)
{

	(void) opcode;
	assert((opcode[0] & 0b11000111) == 0b000000100);
#ifdef VERBOSE
	fprintf(stderr,
			"INR %c\n",
			get_operand_name(GET_DESTINATION_OPERAND(opcode[0])));
#endif
	uint8_t* op_ptr = fetch_operand_ptr(
			GET_DESTINATION_OPERAND(opcode[0]), cpu);
	/* INR increments an 8-bit register or a location in memory.
	 * The aux carry flag will be set if the lower 3 bits of the operator
	 * are set.
	 */
	*op_ptr = _add(*op_ptr, 1, 0, &cpu->flags);

	// If the operand was OPERAND_MEM, then this opcode takes 10 clock
	// cycles. Otherwise, it takes 5.
	(GET_DESTINATION_OPERAND(opcode[0]) == OPERAND_MEM) ? cycle_wait(10)
							    : cycle_wait(5);

	return 1;
}

int dcr(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert((opcode[0] & 0b11000111) == 0b000000101);
#ifdef VERBOSE
	fprintf(stderr,
			"DCR %c\n",
			get_operand_name(GET_DESTINATION_OPERAND(opcode[0])));
#endif
	uint8_t* op_ptr = fetch_operand_ptr(
			GET_DESTINATION_OPERAND(opcode[0]), cpu);
	/* DCR decremtns an 8-bit register or a location in memory.
	 * The aux carry flag will be set iff the lower 4 bits of the operator
	 * are reset.
	 */
	*op_ptr = _add(*op_ptr, -1, 0, &cpu->flags);

	// If the operand was OPERAND_MEM, then this opcode takes 10 clock
	// cycles. Otherwise, it takes 5.
	(GET_DESTINATION_OPERAND(opcode[0]) == OPERAND_MEM) ? cycle_wait(10)
							    : cycle_wait(5);

	return 1;
}

int inx_dcx(const uint8_t* opcode, struct cpu_state* cpu)
{
	(void) opcode;
	assert((opcode[0] & 0b11000111) == 0b00000011);
	/* INX and DCX are increment register pair and derement register pair,
	 * respectively. Bit 3 determines whether it is INX or DCX.
	 */
#ifdef VERBOSE
	fprintf(stderr,
			"%sX %s\n",
			opcode[0] & 0b00001000 ? "DC" : "IN",
			get_register_pair_name_other(opcode[0]));
#endif
	// INX and DCX increment or decrement the register without affecting
	// any condition flags. they take 5 clock cycles and advance the
	// program counter one byte.
	uint16_t* operand = get_register_pair_other(opcode[0], cpu);
	opcode[0] & 0b00001000 ? --*operand : ++*operand;
	cycle_wait(5);
	return 1;
}

int dad(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert((opcode[0] & 0b11001111) == 0b00001001);
#ifdef VERBOSE
	fprintf(stderr, "DAD %s\n", get_register_pair_name_other(opcode[0]));
#endif
	// Add the indicated register pair to HL.
	uint32_t result = cpu->hl + *get_register_pair_other(opcode[0], cpu);
	// Set the carry flag if we carried out of the pair.
	// We don't touch any other flags.
	cpu->flags = (result & (1 << 16)) ? cpu->flags | CARRY_FLAG
					  : cpu->flags & ~CARRY_FLAG;
	cpu->hl = result;
	cycle_wait(10);
	return 1;
}

int daa(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0x27);
#ifdef VERBOSE
	fprintf(stderr, "DAA\n");
#endif

	(void) opcode;
	// If the low nibble > 9, OR aux carry is set, add six to the low
	// nibble.
	uint8_t working = 0;
	if ((cpu->a & 0x0f) > 9 || (cpu->flags & AUX_CARRY_FLAG))
	{ working += 0x06; }
	// If the carry flag is set, OR if the high nibble is above 9, OR
	// if the high nibble is equal to nine AND the low nibble is ABOVE
	// nine, then we add six to the high nibble and set the carry flag.
	//
	// Note that we can SET the CF here, but never clear it.  The exerciser
	// ROM gets cranky if we ever clear it.
	if (((cpu->a & 0xf0) >= 0x90 && (cpu->a & 0x0f) > 9)
			|| (cpu->a & 0xf0) > 0x90 || (cpu->flags & CARRY_FLAG))
	{
		working += 0x60;
		cpu->flags |= CARRY_FLAG;
	}

	working = _add(cpu->a, working, 0, &cpu->flags);
	cpu->a	= working;
	cycle_wait(4);
	return 1;
}
