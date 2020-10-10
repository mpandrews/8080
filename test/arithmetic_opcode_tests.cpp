
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
	// we'll just test that and make sure it works.
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
}
