
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}
#include "gtest/gtest.h"

TEST(Add, BasicCheck)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0x2222, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};
	// ADD A.
	cpu.pc += add_adc(0x87, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Here we expect the Zero and Parity flags to be set, and all others
	// to be cleared;
	EXPECT_EQ(cpu.flags, 0b01000100);
	EXPECT_EQ(cpu.pc, 1);
}

TEST(ADD, AdditionalChecks)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0x2222, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};
	cpu.a = 16;
	cpu.b = 1;
	// ADD B.
	cpu.pc += add_adc(0x80, &cpu);
	EXPECT_EQ(cpu.a, 17);
	EXPECT_EQ(cpu.pc, 1);
	// SZ-A-P-C
	// Only parity should be set.
	EXPECT_EQ(cpu.flags, 0b00000100);

	cpu.h = 8;
	// ADD H.
	cpu.pc += add_adc(0x8c, &cpu);
	EXPECT_EQ(cpu.a, 25);
	// No flags should be set.
	EXPECT_EQ(cpu.flags, 0);
	EXPECT_EQ(cpu.pc, 2);
	// ADD H.
	cpu.pc += add_adc(0x8c, &cpu);
	// Parity and aux carry should be set: bit 3 (8) is set in both
	// operands.
	EXPECT_EQ(cpu.a, 33);
	EXPECT_EQ(cpu.flags, 0b00010100);
	EXPECT_EQ(cpu.pc, 3);

	cpu.l = 255;
	// ADD L
	// This will overflow quite a bit.  Obviously.
	cpu.pc += add_adc(0x85, &cpu);
	EXPECT_EQ(cpu.a, 32);
	// Carry and aux carry flags should be set.
	EXPECT_EQ(cpu.flags, 0b00010001) << cpu.flags;
	EXPECT_EQ(cpu.pc, 4);

	cpu.d = 127;
	// ADD D
	// This will get us 159, which has the high bit set and should therefore
	// set the sign flag.  (Since it's also parseable as -97).
	cpu.pc += add_adc(0x82, &cpu);
	EXPECT_EQ((signed char) cpu.a, -97);
	EXPECT_EQ(cpu.a, 159);
	// Only the sign and parity bits should be set.
	EXPECT_EQ(cpu.flags, 0b10000100);
}

TEST(ADC, All)
{
	// Since ADC differs from ADD only in the treatment of the carry bit,
	// we'll test that and make sure it works.
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0x2222, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};
	// Set the carry bit.
	cpu.flags = 1;
	// ADD A;
	cpu.pc += add_adc(0x87, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero and Parity should be set.
	EXPECT_EQ(cpu.flags, 0b01000100);
	cpu.flags = 1;
	// ADC A;
	cpu.pc += add_adc(0x8f, &cpu);
	EXPECT_EQ(cpu.a, 1);
	// This test is to make sure we're handling situations where the
	// carry bit causes an overflow in the operand itself:
	// we'll use 255/-1, and work with the carry bit set.
	cpu.flags = 1;
	cpu.a	  = 1;
	cpu.b	  = 255;
	cpu.pc += add_adc(0x88, &cpu);
	// 1 + -1 + 1 from carry == 1
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, 0b00000001);
	// And now we make sure we're not spuriously adding in the carry bit
	// when it's not set.
	// A is still 1, B is still 255/-1
	cpu.flags = 0;
	cpu.pc += add_adc(0x88, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// Zero, parity, aux carry, carry.
	EXPECT_EQ(cpu.flags, 0b01010101);
}

TEST(ADI, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0x1200, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	cpu.memory[cpu.pc + 1] = 0xab;

	// ADI
	cpu.pc += adi(0xc6, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0xbd);
	// SZ-A-P-C
	// Sign and parity flags are set
	EXPECT_EQ(cpu.flags, 0b10000100);
}

TEST(ACI, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0x1a01, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	cpu.memory[cpu.pc + 1] = 0xab;

	// ACI
	cpu.pc += aci(0xce, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0xc6);
	// SZ-A-P-C
	// Sign, AC, and parity flags are set
	EXPECT_EQ(cpu.flags, 0b10010100);
}

TEST(SUB, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// SUB A
	cpu.pc += sub_sbb(0x97, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero and Parity should be set.
	EXPECT_EQ(cpu.flags, 0b01000100);

	cpu.a = 1;
	cpu.b = 0;
	// SUB B
	cpu.pc += sub_sbb(0x90, &cpu);
	EXPECT_EQ(cpu.a, 1);
	// No flags should be set.
	EXPECT_EQ(cpu.flags, 0b00000000);

	cpu.a = 0;
	cpu.b = 1;
	cpu.pc += sub_sbb(0x90, &cpu);
	EXPECT_EQ((signed char) cpu.a, -1);
	// Sign, Parity, Carry.  See p. 28 of the programmer's manual.
	EXPECT_EQ(cpu.flags, 0b10000101);

	cpu.a = 16;
	cpu.b = -23;
	cpu.pc += sub_sbb(0x90, &cpu);
	EXPECT_EQ(cpu.a, 39);
	// Parity and Carry.  It might seem like carry shouldn't be set,
	// but operands are always treated as unsigned, so from the perspective
	// of the ALU, -23 is 233.
	EXPECT_EQ(cpu.flags, 0b00000101);

	cpu.a = 8;
	cpu.b = 8;
	cpu.pc += sub_sbb(0x90, &cpu);
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
	EXPECT_EQ(cpu.flags, 0b01010100);
}

TEST(SBB, All)
{
	// To test this, we'll perform one an example from the programming
	// manual: 0x1301 - 0x0503.  It's on page 56.
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	cpu.a = 0x01;
	cpu.b = 0x03;
	// SBB B
	cpu.pc += sub_sbb(0x98, &cpu);
	EXPECT_EQ(cpu.a, 0xfe);
	// SZ-A-P-C
	// Sign, Carry.
	EXPECT_EQ(cpu.flags, 0b10000001);
	cpu.a = 0x13;
	cpu.b = 0x05;
	// Now because the borrow is set, we should get 0x0d.
	cpu.pc += sub_sbb(0x98, &cpu);
	EXPECT_EQ(cpu.a, 0x0d);
	// No flags set.
	EXPECT_EQ(cpu.flags, 0b00000000);
}

TEST(SBI, All)
{

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0, .psw = 0xff00, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// give sbi an argument of 0xff to subtract from the accumulator which
	// is set to 0xff. This should result cpu->a being set to 0x00.
	// Additionally the Zero, Parity, and Aux Carry flags should be set, and
	// the Sign and Carry flags should be reset.
	cpu.memory[0x0001] = 0xff;
	cpu.pc += sui_sbi(0xde, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, 0b01010100);
	//                     SZ-A-P-C

	// Now set the carry flag and perform subtraction that will result in
	// a borrow when the carry flag is added to the subtrahend. Apparently
	// subtrahend is the proper term.
	// subtracting (1(subtrahend) + 1(carry flag)) from 1(contents of cpu->a
	// should result in the accumulator containing 0xff. Additionally
	// the sign and carry flags should be set, and the aux carry, parity,
	// and zero flags should be reset after this operation.
	cpu.a		   = 0x01;
	cpu.memory[0x0003] = 0x01;
	cpu.flags	   = CARRY_FLAG;
	cpu.pc += sui_sbi(0xde, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.flags, 0b10000101);
	//                     SZ-A-P-C
}

TEST(SUI, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0, .psw = 0xff00, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// give sui an argument of 0xfe to subtract from the accumulator which
	// is set to 0xff. This should result in cpu->a being set to 0x01.
	// Additionally the Aux Carry flag should be set, and
	// the Sign, Zero, Parity, and Carry flags should be reset.
	cpu.a		   = 0xff;
	cpu.memory[0x0001] = 0xfe;
	cpu.pc += sui_sbi(0xd6, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, 0b00010000);
	//                     SZ-A-P-C

	// Now set the carry flag and perform subtraction that would result in
	// a borrow when the carry flag is added to the subtrahend, but won't
	// if it isn't included. SBI doesn't include the carry flag. After this
	// operation, the accumulator should contain 0x00. Additionally the
	// Zero, Parity, and Aux Carry flags should be set while the Sign and
	// Carry flags should be reset.
	cpu.a		   = 0x01;
	cpu.memory[0x0003] = 0x01;
	cpu.flags	   = CARRY_FLAG;
	cpu.pc += sui_sbi(0xd6, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, 0b01010100);
	//                     SZ-A-P-C
}

TEST(DAA, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = 0,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// This is the example on page 22 (pdf) of the 1975 Programmer's Manual.
	cpu.a = 0x9B;
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.a, 1);
	// SZ-A-P-C
	EXPECT_EQ(cpu.flags, 0b00010001);

	// The DAAs from page 61 (pdf) of the same manual.

	cpu.a	  = 0xbb;
	cpu.flags = 0; // Clear flags.
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0x21);
	// AC, C, P.
	EXPECT_EQ(cpu.flags, 0b00010101);

	cpu.a	  = 0x73;
	cpu.flags = 0x10; // AC only.
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.a, 0x79);
	// All flags clear.
	EXPECT_EQ(cpu.flags, 0);

	// 1 and 1, with AC set.
	cpu.a	  = 0x11;
	cpu.flags = 0x10;
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0x17);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// 1 and 1, with flags clear.
	cpu.a	  = 0x11;
	cpu.flags = 0;
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 5);
	EXPECT_EQ(cpu.a, 0x11);
	// P only.
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// 1 and 1, with C set.
	cpu.a	  = 0x11;
	cpu.flags = CARRY_FLAG;
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 6);
	EXPECT_EQ(cpu.a, 0x71);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// 1 and 1, A and AC.
	cpu.a	  = 0x11;
	cpu.flags = CARRY_FLAG | AUX_CARRY_FLAG;
	cpu.pc += daa(0x27, &cpu);
	EXPECT_EQ(cpu.pc, 7);
	EXPECT_EQ(cpu.a, 0x77);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);
}

TEST(DAD, All)
{

	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	cpu.bc	  = 0x10;
	cpu.flags = 0xff;
	// DAD B
	cpu.pc += dad(0x09, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.hl, 0x10);
	// Should clear the carry, but only that.
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD H
	cpu.pc += dad(0x29, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.hl, 0x20);
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD SP
	cpu.sp = 0xf0;
	cpu.pc += dad(0x39, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.hl, 0x0110);
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// DAD D, with carry out.
	cpu.hl = 0xf000;
	cpu.de = 0xf000;
	cpu.pc += dad(0x19, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.hl, 0xe000);
	EXPECT_EQ(cpu.flags, 0xff);
}

TEST(INR, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x0,
		.hl = 0x8001, .psw = 0xd5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// First call INR on register B which is set to 0. Assert that it is
	// set to 1 afterwards. Also assert that the pc advanced one byte and
	// that the flags are all cleared, except the carry flag which should
	// be unaffected by INR. All flags are set to one at this point
	cpu.pc += inr(0x04, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.b, 1);
	EXPECT_EQ(cpu.flags, 0b00000001);
	//                     SZ-A-P-C

	// Next, call INR on register C which will be set to 0x8f. This should
	// leave register C set to 0x90, advance the pc by 1, and set the aux
	// carry, parity, and sign flags.
	cpu.c = 0x8f;
	cpu.pc += inr(0x0c, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.c, 0x90);
	EXPECT_EQ(cpu.flags, 0b10010101);

	// Call INR with MEM operand, which will hold the value 0xff. This
	// should leave 0x00 in memory at the address pointed to by HL, should
	// set the zero, aux carry, and parity flags.
	cpu.memory[0x8001] = 0xff;
	cpu.pc += inr(0x34, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.memory[cpu.hl], 0x00);
	EXPECT_EQ(cpu.flags, 0b01010101);

	// Finally, manually reset the carry flag, cause a carry, and assert
	// that INR did not set the carry flag. Additionally, the zero, aux
	// carry, and parity flags should be set.
	cpu.flags = 0;
	cpu.a	  = 0xff;
	cpu.pc += inr(0x3c, &cpu);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.flags, 0b01010100);
}

TEST(DCR, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x0,
		.hl = 0x8001, .psw = 0xd5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// First decrement the A register which will be set to 0x02. Then,
	// assert that it holds the value 1 afterwards. Additionally, the
	// sign, zero, and parity flags should be reset. the aux carry flag
	// should remain set. DCR does not affect the carry flag.
	cpu.a = 2;
	cpu.pc += dcr(0x3d, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, 0b00010001);
	//                     SZ-A-P-C

	// Now set byte 0x8001 in memory (pointed to by register hl) to 0x00.
	// Then call dcr mem and assert that it now holds the value 0xff. Also
	// assert that the sign and parity flags are set, and aux carry is
	// reset.
	cpu.memory[0x8001] = 0x00;
	cpu.pc += dcr(0x35, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.memory[cpu.hl], 0xff);
	EXPECT_EQ(cpu.flags, 0b10000101);

	// Now set the d register to hold the value 1 and decrement it. This
	// should leave D register at 0. It should also set the zero flag, the
	// parity flag, and the aux carry flag. It should reset the sign flag.
	cpu.d = 1;
	cpu.pc += dcr(0x15, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.d, 0x00);
	EXPECT_EQ(cpu.flags, 0b01010101);

	// Lastly, reset the flags manually, induce a carry(borrow) and assert
	// that the carry flag and aux carry flags were not set. The zero and
	// parity should be set by this operation.
	cpu.flags = 0;
	cpu.c	  = 0x00;
	cpu.pc += dcr(0x0d, &cpu);
	EXPECT_EQ(cpu.c, 0xff);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.flags, 0b10000100);
}
