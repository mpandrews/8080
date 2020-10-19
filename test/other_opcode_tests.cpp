
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

TEST(EI, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// EI
	cpu.pc += ei(0xFB, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.interrupt_enable_flag, 2);
}

TEST(DI, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// DI
	cpu.pc += di(0xF3, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.interrupt_enable_flag, 0);
}

TEST(HLT, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// HLT
	EXPECT_EQ(hlt(0x76, &cpu), 1);
	EXPECT_EQ(cpu.halt_flag, 1);
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
	cpu.pc += nop(0x00, &cpu);
	EXPECT_EQ(cpu.pc, 1);

	// NOP 0x38
	cpu.pc += nop(0x38, &cpu);
	EXPECT_EQ(cpu.pc, 2);
}

TEST(XTHL, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	// set the contents of the h register, and the stack pointer
	// and put something in memory at the addres pointed to by the
	// stack pointer.
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0,
		.sp = 0x10ad, .pc = 0, .bc = 0, .de = 0, .hl = 0x0b3c, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	cpu.memory[cpu.sp]     = 0xf0;
	cpu.memory[cpu.sp + 1] = 0x0d;

	// execute xthl, assert that the contents of the h register are now
	// stored in memory at the address pointed to by the stack pointer
	// and vice versa
	cpu.pc += xthl(0xe3, &cpu);
	EXPECT_EQ(cpu.sp, 0x10ad);
	EXPECT_EQ(cpu.memory[cpu.sp], 0x3c);
	EXPECT_EQ(cpu.memory[cpu.sp + 1], 0x0b);
	EXPECT_EQ(cpu.h, 0x0d);
	EXPECT_EQ(cpu.l, 0xf0);
}

TEST(SPHL, All)
{
	// set the contents of the h register, and the stack pointer
	// and put something in memory at the addres pointed to by the
	// stack pointer.
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = nullptr,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0,
		.sp = 0x10ad, .pc = 0, .bc = 0, .de = 0, .hl = 0x0b3c, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	cpu.pc += sphl(0xf9, &cpu);
	EXPECT_EQ(cpu.sp, 0x0b3c);
	EXPECT_EQ(cpu.hl, 0x0b3c);
}

TEST(PUSH, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0,
		.sp = 0x1010, .pc = 0, .bc = 0x0102, .de = 0x0304, .hl = 0x0506,
		.psw = 0xfffd, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};
	// PUSH B
	cpu.pc += push(0xc5, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.sp, 0x100e); // 0x1010 - 2
	EXPECT_EQ(cpu.memory[cpu.sp], 0x02);
	EXPECT_EQ(cpu.memory[cpu.sp + 1], 0x01);

	// PUSH D
	cpu.pc += push(0xd5, &cpu);
	EXPECT_EQ(cpu.sp, 0x100c);
	EXPECT_EQ(*(uint16_t*) (cpu.memory + cpu.sp), 0x0304);

	// PUSH H
	cpu.pc += push(0xe5, &cpu);
	EXPECT_EQ(cpu.sp, 0x100a);
	EXPECT_EQ(*(uint16_t*) (cpu.memory + cpu.sp), 0x0506);

	// PUSH PSW
	cpu.pc += push(0xf5, &cpu);
	EXPECT_EQ(cpu.sp, 0x1008);
	// Bits 1, 3 and 5 of PSW always push to the same values: 1, 0, and 0.
	// The value we currently have in PSW has those flipped: the flag byte
	// is 0xfd.  The value pushed should be 0xd7.
	EXPECT_EQ(*(uint16_t*) (cpu.memory + cpu.sp), 0xffd7);
}
