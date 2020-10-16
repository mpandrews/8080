
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
	EXPECT_EQ(4, add_adc(0x87, &cpu));
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
	add_adc(0x80, &cpu);
	EXPECT_EQ(cpu.a, 17);
	EXPECT_EQ(cpu.pc, 1);
	// SZ-A-P-C
	// Only parity should be set.
	EXPECT_EQ(cpu.flags, 0b00000100);

	cpu.h = 8;
	// ADD H.
	add_adc(0x8c, &cpu);
	EXPECT_EQ(cpu.a, 25);
	// No flags should be set.
	EXPECT_EQ(cpu.flags, 0);
	EXPECT_EQ(cpu.pc, 2);
	// ADD H.
	add_adc(0x8c, &cpu);
	// Parity and aux carry should be set: bit 3 (8) is set in both
	// operands.
	EXPECT_EQ(cpu.a, 33);
	EXPECT_EQ(cpu.flags, 0b00010100);
	EXPECT_EQ(cpu.pc, 3);

	cpu.l = 255;
	// ADD L
	// This will overflow quite a bit.  Obviously.
	add_adc(0x85, &cpu);
	EXPECT_EQ(cpu.a, 32);
	// Carry and aux carry flags should be set.
	EXPECT_EQ(cpu.flags, 0b00010001) << cpu.flags;
	EXPECT_EQ(cpu.pc, 4);

	cpu.d = 127;
	// ADD D
	// This will get us 159, which has the high bit set and should therefore
	// set the sign flag.  (Since it's also parseable as -97).
	add_adc(0x82, &cpu);
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
	add_adc(0x87, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero and Parity should be set.
	EXPECT_EQ(cpu.flags, 0b01000100);
	cpu.flags = 1;
	// ADC A;
	add_adc(0x8f, &cpu);
	EXPECT_EQ(cpu.a, 1);
	// This test is to make sure we're handling situations where the
	// carry bit causes an overflow in the operand itself:
	// we'll use 255/-1, and work with the carry bit set.
	cpu.flags = 1;
	cpu.a	  = 1;
	cpu.b	  = 255;
	add_adc(0x88, &cpu);
	// 1 + -1 + 1 from carry == 1
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, 0b00000001);
	// And now we make sure we're not spuriously adding in the carry bit
	// when it's not set.
	// A is still 1, B is still 255/-1
	cpu.flags = 0;
	add_adc(0x88, &cpu);
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
	int cycles = adi(0xc6, &cpu);
	EXPECT_EQ(cycles, 7);
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
	int cycles = aci(0xce, &cpu);
	EXPECT_EQ(cycles, 7);
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
	sub_sbb(0x97, &cpu);
	EXPECT_EQ(cpu.a, 0);
	// SZ-A-P-C
	// Zero and Parity should be set.
	EXPECT_EQ(cpu.flags, 0b01000100);

	cpu.a = 1;
	cpu.b = 0;
	// SUB B
	sub_sbb(0x90, &cpu);
	EXPECT_EQ(cpu.a, 1);
	// No flags should be set.
	EXPECT_EQ(cpu.flags, 0b00000000);

	cpu.a = 0;
	cpu.b = 1;
	sub_sbb(0x90, &cpu);
	EXPECT_EQ((signed char) cpu.a, -1);
	// Sign, Parity, Carry.  See p. 28 of the programmer's manual.
	EXPECT_EQ(cpu.flags, 0b10000101);

	cpu.a = 16;
	cpu.b = -23;
	sub_sbb(0x90, &cpu);
	EXPECT_EQ(cpu.a, 39);
	// Parity and Carry.  It might seem like carry shouldn't be set,
	// but operands are always treated as unsigned, so from the perspective
	// of the ALU, -23 is 233.
	EXPECT_EQ(cpu.flags, 0b00000101);

	cpu.a = 8;
	cpu.b = 8;
	sub_sbb(0x90, &cpu);
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
	sub_sbb(0x98, &cpu);
	EXPECT_EQ(cpu.a, 0xfe);
	// SZ-A-P-C
	// Sign, Carry.
	EXPECT_EQ(cpu.flags, 0b10000001);
	cpu.a = 0x13;
	cpu.b = 0x05;
	// Now because the borrow is set, we should get 0x0d.
	sub_sbb(0x98, &cpu);
	EXPECT_EQ(cpu.a, 0x0d);
	// No flags set.
	EXPECT_EQ(cpu.flags, 0b00000000);
}
