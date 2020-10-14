
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

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
