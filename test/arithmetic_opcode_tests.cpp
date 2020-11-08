
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_size.h"
}
#include "gtest/gtest.h"

TEST(Add, BasicCheck)
{
	struct cpu_state cpu
	{
		.de = 0x1111, .hl = 0x2222,
	};
	// ADD A.
	uint8_t opcode = 0x87;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Here we expect the Zero and Parity flags to be set, and all others
	// to be cleared;
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG);
}

TEST(ADD, AdditionalChecks)
{
	struct cpu_state cpu
	{
		.de = 0x1111, .hl = 0x2222,
	};
	cpu.a = 16;
	cpu.b = 1;
	// ADD B.
	uint8_t opcode = 0x80;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 17);
	// SZ-A-P-C
	// Only parity should be set.
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	cpu.h = 8;
	// ADD H.
	opcode = 0x8c;
	add_adc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 25);
	// No flags should be set.
	EXPECT_EQ(cpu.flags, 0);
	// ADD H.
	opcode = 0x8c;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	// Parity and aux carry should be set: bit 3 (8) is set in both
	// operands.
	EXPECT_EQ(cpu.a, 33);
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG | PARITY_FLAG);

	cpu.l = 255;
	// ADD L
	// This will overflow quite a bit.  Obviously.
	opcode = 0x85;
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(cpu.a, 32);
	// Carry and aux carry flags should be set.
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG | CARRY_FLAG);

	cpu.d = 127;
	// ADD D
	// This will get us 159, which has the high bit set and should therefore
	// set the sign flag.  (Since it's also parseable as -97).
	opcode = 0x82;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ((signed char) cpu.a, -97);
	EXPECT_EQ(cpu.a, 159);
	// Only the sign and parity bits should be set.
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);
}

TEST(ADC, All)
{
	// Since ADC differs from ADD only in the treatment of the carry bit,
	// we'll test that and make sure it works.
	struct cpu_state cpu
	{
		.de = 0x1111, .hl = 0x2222,
	};
	// Set the carry bit.
	cpu.flags = 1;
	// ADD A;
	uint8_t opcode = 0x87;
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero and Parity should be set.
	EXPECT_EQ(cpu.flags, 0b01000100);
	cpu.flags = 1;
	// ADC A;
	opcode = 0x8f;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	add_adc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 1);
	// This test is to make sure we're handling situations where the
	// carry bit causes an overflow in the operand itself:
	// we'll use 255/-1, and work with the carry bit set.
	cpu.flags = 1;
	cpu.a	  = 1;
	cpu.b	  = 255;
	opcode	  = 0x88;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	// 1 + -1 + 1 from carry == 1
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, CARRY_FLAG | AUX_CARRY_FLAG);
	// And now we make sure we're not spuriously adding in the carry bit
	// when it's not set.
	// A is still 1, B is still 255/-1
	cpu.flags = 0;
	EXPECT_EQ(add_adc(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0);
	// Zero, parity, aux carry, carry.
	EXPECT_EQ(cpu.flags,
			ZERO_FLAG | PARITY_FLAG | AUX_CARRY_FLAG | CARRY_FLAG);
}

TEST(ADI, All)
{
	struct cpu_state cpu
	{
		.psw = 0x1200
	};

	// ADI
	uint8_t opcode[2] = {0xc6, 0xab};

	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(adi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0xbd);
	// SZ-A-P-C
	// Sign and parity flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);
}

TEST(ACI, All)
{
	struct cpu_state cpu
	{
		.psw = 0x1a01
	};

	uint8_t opcode[2] = {0xce, 0xab};

	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	// ACI
	EXPECT_EQ(aci(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0xc6);
	// SZ-A-P-C
	// Sign, AC, and parity flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(SUB, All)
{
	struct cpu_state cpu
	{
		0
	};

	// SUB A
	uint8_t opcode = 0x97;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero, Parity, and AC should be set.
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG | AUX_CARRY_FLAG);

	cpu.a = 1;
	cpu.b = 0;
	// SUB B
	opcode = 0x90;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 1);
	// AC should be set.
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG);

	cpu.a = 0;
	cpu.b = 1;
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ((signed char) cpu.a, -1);
	// Sign, Parity, Carry.  See p. 28 of the programmer's manual.
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG | CARRY_FLAG);

	cpu.a = 16;
	cpu.b = -23;
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 39);
	// Parity and Carry.  It might seem like carry shouldn't be set,
	// but operands are always treated as unsigned, so from the perspective
	// of the ALU, -23 is 233.
	EXPECT_EQ(cpu.flags, PARITY_FLAG | CARRY_FLAG);

	cpu.a = 8;
	cpu.b = 8;
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0);
	/*Zero, Aux Carry, Parity.
	 * This may also require some explanation.
	 * 8      = 0000 1000
	 * ~8 + 1 = 1111 1000
	 * Note that bit 3 is set in both: that means the aux carry will
	 * fire, since it does not behave differently for addition and
	 * subtraction.  I think.  Different emulators do it differently,
	 * Intel's documentation is ambiguous, and I don't have a real chip to
	 * test on.
	 */
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(SBB, All)
{
	// To test this, we'll perform one an example from the programming
	// manual: 0x1301 - 0x0503.  It's on page 56.
	struct cpu_state cpu
	{
		0
	};

	cpu.a = 0x01;
	cpu.b = 0x03;
	// SBB B
	uint8_t opcode = 0x98;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0xfe);
	// SZ-A-P-C
	// Sign, Carry.
	EXPECT_EQ(cpu.flags, SIGN_FLAG | CARRY_FLAG);
	cpu.a = 0x13;
	cpu.b = 0x05;
	// Now because the borrow is set, we should get 0x0d.
	EXPECT_EQ(sub_sbb(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 0x0d);
	// No flags set.
	EXPECT_EQ(cpu.flags, 0b00000000);
}

TEST(SBI, All)
{

	struct cpu_state cpu
	{
		0
	};
	cpu.de	= 0x1111;
	cpu.psw = 0xff00;

	// give sbi an argument of 0xff to subtract from the accumulator which
	// is set to 0xff. This should result cpu->a being set to 0x00.
	// Additionally the Zero, Parity, and Aux Carry flags should be set, and
	// the Sign and Carry flags should be reset.
	uint8_t opcode[2] = {0xde, 0xff};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(sui_sbi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);

	// Now set the carry flag and perform subtraction that will result in
	// a borrow when the carry flag is added to the subtrahend. Apparently
	// subtrahend is the proper term.
	// subtracting (1(subtrahend) + 1(carry flag)) from 1(contents of cpu->a
	// should result in the accumulator containing 0xff. Additionally
	// the sign and carry flags should be set, and the aux carry, parity,
	// and zero flags should be reset after this operation.
	cpu.a	  = 0x01;
	opcode[1] = 0x01;
	cpu.flags = CARRY_FLAG;
	EXPECT_EQ(sui_sbi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG | CARRY_FLAG);
}

TEST(SUI, All)
{
	struct cpu_state cpu
	{
		0
	};
	cpu.de	= 0x1111;
	cpu.psw = 0xff00;

	// give sui an argument of 0xfe to subtract from the accumulator which
	// is set to 0xff. This should result in cpu->a being set to 0x01.
	// Additionally the Aux Carry flag should be set, and
	// the Sign, Zero, Parity, and Carry flags should be reset.
	uint8_t opcode[2] = {0xd6, 0xfe};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	cpu.a = 0xff;
	EXPECT_EQ(sui_sbi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG);

	// Now set the carry flag and perform subtraction that would result in
	// a borrow when the carry flag is added to the subtrahend, but won't
	// if it isn't included. SBI doesn't include the carry flag. After this
	// operation, the accumulator should contain 0x00. Additionally the
	// Zero, Parity, and Aux Carry flags should be set while the Sign and
	// Carry flags should be reset.
	cpu.a	  = 0x01;
	opcode[1] = 0x01;
	cpu.flags = CARRY_FLAG;
	EXPECT_EQ(sui_sbi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(DAA, All)
{
	struct cpu_state cpu
	{
		0
	};

	// This is the example on page 22 (pdf) of the 1975 Programmer's Manual.
	cpu.a	       = 0x9B;
	uint8_t opcode = 0x27;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(daa(&opcode, &cpu), 4);
	EXPECT_EQ(cpu.a, 1);
	// SZ-A-P-C
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG | CARRY_FLAG);

	// The DAAs from page 61 (pdf) of the same manual.

	cpu.a	  = 0xbb;
	cpu.flags = 0; // Clear flags.
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x21);
	// AC, C, P.
	EXPECT_EQ(cpu.flags, 0b00010101);

	cpu.a	  = 0x73;
	cpu.flags = 0x10; // AC only.
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x79);
	// All flags clear.
	EXPECT_EQ(cpu.flags, 0);

	// 1 and 1, with AC set.
	cpu.a	  = 0x11;
	cpu.flags = 0x10;
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x17);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// 1 and 1, with flags clear.
	cpu.a	  = 0x11;
	cpu.flags = 0;
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x11);
	// P only.
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// 1 and 1, with C set.
	cpu.a	  = 0x11;
	cpu.flags = CARRY_FLAG;
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x71);
	EXPECT_EQ(cpu.flags, PARITY_FLAG | CARRY_FLAG);

	// 1 and 1, A and AC.
	cpu.a	  = 0x11;
	cpu.flags = CARRY_FLAG | AUX_CARRY_FLAG;
	daa(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x77);
	EXPECT_EQ(cpu.flags, PARITY_FLAG | CARRY_FLAG);
}

TEST(DAD, All)
{

	struct cpu_state cpu
	{
		0
	};

	cpu.bc	  = 0x10;
	cpu.flags = 0xff;
	// DAD B
	uint8_t opcode = 0x09;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(dad(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.hl, 0x10);
	// Should clear the carry, but only that.
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD H
	opcode = 0x29;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(dad(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.hl, 0x20);
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD SP
	cpu.sp = 0xf0;
	opcode = 0x39;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(dad(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.hl, 0x0110);
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD D, with carry out.
	cpu.hl = 0xf000;
	cpu.de = 0xf000;
	opcode = 0x19;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(dad(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.hl, 0xe000);
	EXPECT_EQ(cpu.flags, 0xff);
}

TEST(INR, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	unsigned char rom_mask = 0;
	struct cpu_state cpu
	{
		.memory = memory, .rom_mask = &rom_mask, .mask_shift = 16,
	};
	cpu.hl	= 0x8001;
	cpu.psw = 0xd5;

	// First call INR on register B which is set to 0. Assert that it is
	// set to 1 afterwards. Also assert that the pc advanced one byte and
	// that the flags are all cleared, except the carry flag which should
	// be unaffected by INR. All flags are set to one at this point
	uint8_t opcode = 0x04;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inr(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.b, 1);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);

	// Next, call INR on register C which will be set to 0x8f. This should
	// leave register C set to 0x90, advance the pc by 1, and set the aux
	// carry, parity, and sign flags.
	cpu.c  = 0x8f;
	opcode = 0x0c;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	inr(&opcode, &cpu);
	EXPECT_EQ(cpu.c, 0x90);
	EXPECT_EQ(cpu.flags,
			SIGN_FLAG | AUX_CARRY_FLAG | PARITY_FLAG | CARRY_FLAG);

	// Call INR with MEM operand, which will hold the value 0xff. This
	// should leave 0x00 in memory at the address pointed to by HL, should
	// set the zero, aux carry, and parity flags.
	cpu.memory[0x8001] = 0xff;
	opcode		   = 0x34;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inr(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.memory[cpu.hl], 0x00);
	EXPECT_EQ(cpu.flags,
			ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG | CARRY_FLAG);

	// Finally, manually reset the carry flag, cause a carry, and assert
	// that INR did not set the carry flag. Additionally, the zero, aux
	// carry, and parity flags should be set.
	cpu.flags = 0;
	cpu.a	  = 0xff;
	opcode	  = 0x3c;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	inr(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(DCR, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	unsigned char rom_mask = 0;
	struct cpu_state cpu
	{
		.memory = memory, .rom_mask = &rom_mask, .mask_shift = 16,
	};
	cpu.hl	= 0x8001;
	cpu.psw = 0xd5;
	// First decrement the A register which will be set to 0x02. Then,
	// assert that it holds the value 1 afterwards. Additionally, the
	// sign, zero, and parity flags should be reset. the aux carry flag
	// should remain set. DCR does not affect the carry flag.
	cpu.a = 2;

	uint8_t opcode = 0x3d;
	EXPECT_EQ(get_opcode_size(opcode), 1);

	EXPECT_EQ(dcr(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG | CARRY_FLAG);

	// Now set byte 0x8001 in memory (pointed to by register hl) to 0x00.
	// Then call dcr mem and assert that it now holds the value 0xff. Also
	// assert that the sign and parity flags are set, and aux carry is
	// reset.
	cpu.memory[0x8001] = 0x00;
	opcode		   = 0x35;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(dcr(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.memory[cpu.hl], 0xff);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG | CARRY_FLAG);

	// Now set the d register to hold the value 1 and decrement it. This
	// should leave D register at 0. It should also set the zero flag, the
	// parity flag, and the aux carry flag. It should reset the sign flag.
	cpu.d  = 1;
	opcode = 0x15;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	dcr(&opcode, &cpu);
	EXPECT_EQ(cpu.d, 0x00);
	EXPECT_EQ(cpu.flags,
			ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG | CARRY_FLAG);

	// Lastly, reset the flags manually, induce a carry(borrow) and assert
	// that the carry flag and aux carry flags were not set. The sign and
	// parity should be set by this operation.
	cpu.flags = 0;
	cpu.c	  = 0x00;

	opcode = 0x0d;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	dcr(&opcode, &cpu);
	EXPECT_EQ(cpu.c, 0xff);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);
}

TEST(DCX, All)
{
	struct cpu_state cpu
	{
		0
	};
	cpu.de = 0x8000;
	cpu.hl = 1;

	// Decrement the bc register. Since it is 0 right now, we should expect
	// it to be 0xffff after the instruction. No condition flags are
	// affected by DCX
	uint8_t opcode = 0x0b;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.bc, 0xffff);
	EXPECT_EQ(cpu.flags, 0x00);

	// Decrement the hl register. Since it is 1, it should be 0 afterwards.
	// This time, the flags register will be set to 0xff, and it should
	// remain at 0xff after the instruction.
	cpu.flags = 0xff;
	opcode	  = 0x2b;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.hl, 0);
	EXPECT_EQ(cpu.flags, 0xff);

	// Decrement the de register. It is set to 0x8000, so we should expect
	// it to contain 0x7fff afterwards.
	opcode = 0x1b;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.de, 0x7fff);
	EXPECT_EQ(cpu.flags, 0xff);

	// And lastly, we'll decrement the SP because it's the only one left and
	// we might as well. This test will set the flags back to 0 and assert
	// that are set by the operation.
	cpu.sp	  = 0x0100;
	cpu.flags = 0;
	opcode	  = 0x3b;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.sp, 0x00ff);
	EXPECT_EQ(cpu.flags, 0);
}

TEST(INX, All)
{
	struct cpu_state cpu
	{
		0
	};
	cpu.bc = 0xffff;
	cpu.de = 0x7fff;

	// Increment the BC register. Since it is 0xffff right now, we should
	// expect it to be 0x0000 after the instruction. No condition flags are
	// affected by DCX
	uint8_t opcode = 0x03;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.bc, 0x0000);
	EXPECT_EQ(cpu.flags, 0x00);

	// Increment the HL register. Since it is 0, it should be 1 afterwards.
	// This time, the flags register will be set to 0xff, and it should
	// remain at 0xff after the instruction.
	cpu.flags = 0xff;
	opcode	  = 0x23;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.hl, 1);
	EXPECT_EQ(cpu.flags, 0xff);

	// Increment the de register. It is set to 0x7fff, so we should expect
	// it to contain 0x8000 afterwards.
	opcode = 0x13;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.de, 0x8000);
	EXPECT_EQ(cpu.flags, 0xff);

	// And lastly, we'll increment the SP because it's the only one left and
	// we might as well. This test will set the flags back to 0 and assert
	// that are set by the operation.
	cpu.sp	  = 0x0200;
	cpu.flags = 0;
	opcode	  = 0x33;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(inx_dcx(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.sp, 0x0201);
	EXPECT_EQ(cpu.flags, 0);
}
