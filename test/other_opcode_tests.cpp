
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

TEST(POP, B_D_H_PSW)
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
	memset(memory, 0, 1 << 16);

	// POP B
	cpu.memory[cpu.sp]     = 0x12;
	cpu.memory[cpu.sp + 1] = 0x34;
	pop(0xc1, &cpu);
	EXPECT_EQ(cpu.bc, 0x3412);
	EXPECT_EQ(cpu.sp, 2);

	// POP D
	cpu.memory[cpu.sp]     = 0x12;
	cpu.memory[cpu.sp + 1] = 0x34;
	pop(0xd1, &cpu);
	EXPECT_EQ(cpu.de, 0x3412);
	EXPECT_EQ(cpu.sp, 4);

	// POP H
	cpu.memory[cpu.sp]     = 0x12;
	cpu.memory[cpu.sp + 1] = 0x34;
	pop(0xe1, &cpu);
	EXPECT_EQ(cpu.hl, 0x3412);
	EXPECT_EQ(cpu.sp, 6);

	// POP PSW
	cpu.memory[cpu.sp]     = 0x12;
	cpu.memory[cpu.sp + 1] = 0x34;
	pop(0xf1, &cpu);
	EXPECT_EQ(cpu.psw, 0x3412);
	EXPECT_EQ(cpu.sp, 8);
}

TEST(NOP, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// NOP 0x00
	EXPECT_EQ(nop(0x00, &cpu), 4);
	EXPECT_EQ(cpu.pc, 1);

	// NOP 0x38
	EXPECT_EQ(nop(0x38, &cpu), 4);
	EXPECT_EQ(cpu.pc, 2);
}

TEST(XTHL, all)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	cpu.h = 0xff;
	cpu.l = 0xfe;
}
