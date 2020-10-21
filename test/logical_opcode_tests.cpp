
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

TEST(CMC, All)
{
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = nullptr,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0x00ff,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	// With all the flags set, call CMC and assert that just the Carry flag
	// was reset and all other flags were unaffected
	cpu.pc += cmc(0x3f, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.flags, 0xfe);

	// Now that the carry flag is reset, call CMC again and assert that the
	// flags register is now 0xff again
	cpu.pc += cmc(0X3f, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0xff);
}

TEST(CMA, All)
{
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = nullptr,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// cpu.a is set to 0 to start, assert that it is complemented to 0xff
	cpu.pc += cma(0x2f, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.a, 0xff);

	// now that it is 0xff, complement it again and asser that it has been
	// reset to 0x00
	cpu.pc += cma(0X2f, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0);
}

TEST(ANA, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0x8222, .psw = 0x00c5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// This will clear the high four bits of cpu.a and leave the low four
	// bits unchanged resulting in 0x0d. This should also reset the sign,
	// zero, parity, and carry bits (all of which were set in the
	// cpu_state above). It should set the aux carry bit which was reset
	// in the cpu_state above.
	cpu.a = 0xfd;
	cpu.c = 0x0f;
	// ANA C
	cpu.pc += ana(0xa1, &cpu);
	EXPECT_EQ(cpu.a, 0x0d);
	EXPECT_EQ(cpu.pc, 1);
	// only the aux carry bit should be set because either (in this case
	// both) operand has bit 3 set
	EXPECT_EQ(cpu.flags, 0b00010000);
	//                     SZ-A-P-C

	// Now set register A to 0xf7 and put 0b10000001 in memory at 0x8222,
	// the address ponited to by cpu.hl. We'll call ANA M this time to
	// assert that 7 cycles instead of 4 are returned and the other ANA
	// operations also work as expected. The sign and parity flags
	// should be set after this operation. The Aux Carry flag should be
	// reset because neither 0xf7 or 0x81 sets bit 3 high.
	cpu.a		   = 0xf7;
	cpu.memory[0x8222] = 0x81;
	cpu.pc += ana(0xa6, &cpu);
	EXPECT_EQ(cpu.a, 0x81);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Lastly, we'll zero-out the C register, set the A register to 0xff,
	// AND the two, and assert that A == 0 and the zero bit is set by
	// ANA. The parity and aux carry flags should also be set by this
	// operation.
	cpu.c = 0x00;
	cpu.a = 0xff;
	cpu.pc += ana(0xa1, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, 0b01010100);
	//                     SZ-A-P-C
}

TEST(ORA, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0x8222, .psw = 0x00d5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// ORA against C register. This should set all 4 low bits of the A
	// register and leave the high 4 bits unaltered. All flags shouild be
	// cleared (all were set high in the struct_cpu instantiation above),
	cpu.a = 0x23;
	cpu.c = 0x0f;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0x2f);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.flags, 0b00000000);
	//                     SZ-A-P-C

	// For this test, zero-out the A register and put 0b11101101 in
	// memory at 0x8222, the address ponited to by cpu.hl. We'll call ORA M
	// this time to assert that 7 cycles instead of 4 are returned and the
	// other ORA operations also work as expected. the sign and parity flags
	// should be set after this operation. Other flags should be reset.
	cpu.a		   = 0x00;
	cpu.memory[0x8222] = 0xed;
	cpu.pc += ora(0xb6, &cpu);
	EXPECT_EQ(cpu.a, 0xed);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Lastly, zero-out the A register, set the C register to 0xff, OR the
	// two and assert that A is set to 0xff
	cpu.a = 0x00;
	cpu.c = 0xff;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Actually, just one more test to make sure the zero flag is set
	// when it ought to be set by ORA
	cpu.a = 0x00;
	cpu.c = 0x00;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.flags, 0b01000100);
	//                     SZ-A-P-C
}

TEST(STC, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// STC should set the carry flag if it's not set
	cpu.pc += stc(0x37, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.flags, 0x00000001);

	// Call STC again and check that carry flag is still set
	stc(0x37, &cpu);
	EXPECT_EQ(cpu.flags, 0x00000001);
}

TEST(ORI, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	/* Set an argument in memory to OR the accumulator with. The argument
	 * 0x73 should set the accumulator to equal 0x73. Additionally, this
	 * should reset all the flags (all of which are set before the
	 * operation).
	 */
	memory[1] = 0x73;
	cpu.flags = 0b11010101; // all flags are set
	cpu.pc += ori(0xf6, &cpu);
	EXPECT_EQ(cpu.a, 0x73);
	EXPECT_EQ(cpu.pc, 0x02);
	EXPECT_EQ(cpu.flags, 0b00000000);
	//                     SZ-A-P-C

	/* Now OR the accumulator with 0xf0. This should leave the accumulator
	 * with the value 0xf3. This should aslo set the parity and sign flags
	 */
	memory[3] = 0xf0;
	cpu.pc += ori(0xf6, &cpu);
	EXPECT_EQ(cpu.a, 0xf3);
	EXPECT_EQ(cpu.pc, 0x04);
	EXPECT_EQ(cpu.flags, 0b10000100);

	/* Lastly, zero-out the accumulator and OR it with 0. This should leave
	 * the A register set to 0 and should set the Zero and Parity flags
	 */
	cpu.a = 0;
	cpu.pc += ori(0xf6, &cpu);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.pc, 0x06);
	EXPECT_EQ(cpu.flags, 0b01000100);
}

TEST(CPI, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x0,
		.hl = 0x8001, .psw = 0x02d5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	/* The accumulator is set equal to 2 and all the flags are turned on
	 * at this point. This will compare the accumulator's value against 1.
	 * This should reset the sign, zero, aux carry, parity, and carry flags
	 */

	cpu.memory[1] = 1;
	cpu.pc += cpi(0xfe, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0b00010000);
	//                     SZ-A-P-C

	/* Now we'll compare the accumulator against an equal value. This should
	 * set the Zero flag and leave the rest of the flags reset
	 */
	cpu.memory[3] = 2;
	cpu.pc += cpi(0xfe, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.flags, 0b01010100);

	/* Now we'll compare 2 to 3, this should set the Sign, Aux carry,
	 * and Carry flags and it should reset the zero and parity flags
	 */
	cpu.memory[5] = 3;
	cpu.pc += cpi(0xfe, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.pc, 6);
	EXPECT_EQ(cpu.flags, 0b10000101);
}
