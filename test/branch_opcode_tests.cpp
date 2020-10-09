
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

TEST(JMP, All)
{
	unsigned char memory[(1 << 16)];
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// JMP <0x3412>
	cpu.memory[cpu.pc + 1] = 0x12;
	cpu.memory[cpu.pc + 2] = 0x34;
	jmp(0xc3, &cpu);
	EXPECT_EQ(cpu.pc, 0x3412);

	// JMP <0xbbaa>
	cpu.memory[cpu.pc + 1] = 0xaa;
	cpu.memory[cpu.pc + 2] = 0xbb;
	jmp(0xcb, &cpu);
	EXPECT_EQ(cpu.pc, 0xbbaa);
}
