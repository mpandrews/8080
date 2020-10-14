
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

	// with all the flags set, call cmc and assert that just the carry flag
	// was reset and all other flags were unaffected
	int cycles = cmc(0x3f, &cpu);
	EXPECT_EQ(cycles, 1);
	EXPECT_EQ(cpu.flags, (uint8_t) ~CARRY_FLAG);

	// now that the carry flag is reset, call it again and asser that the
	// flags register is now all set again and all other flags are still set
	cycles = cmc(0X3f, &cpu);
	EXPECT_EQ(cycles, 1);
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
	int cycles = cma(0x2f, &cpu);
	EXPECT_EQ(cycles, 1);
	EXPECT_EQ(cpu.a, 0xff);

	// now that it is 0xff, complement it again and asser that it has been
	// reset to 0x00
	cycles = cma(0X2f, &cpu);
	EXPECT_EQ(cycles, 1);
	EXPECT_EQ(cpu.a, 0);
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
	int cycles = stc(0x37, &cpu);
	EXPECT_EQ(cycles, 4);
	EXPECT_EQ(cpu.flags, 0x00000001);

	// Call STC again and check that carry flag is still set
	stc(0x37, &cpu);
	EXPECT_EQ(cpu.flags, 0x00000001);
}
