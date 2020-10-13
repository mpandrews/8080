
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

TEST(MOV, RegisterToRegister)
{
	struct cpu_state cpu
	{};
	cpu.bc = 0;
	cpu.de = 0xFFFF;
	cpu.hl = 0;

	// MOV B,D
	mov(0x42, &cpu);
	EXPECT_EQ(cpu.bc, 0xFF00);

	// MOV H,B
	mov(0x60, &cpu);
	// MOV L,B
	mov(0x68, &cpu);

	EXPECT_EQ(cpu.hl, 0xFFFF);
}

TEST(MOV, RegisterToMem)
{

	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0x0001, .de = 0, .hl = 12345, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	memset(memory, 0, MAX_MEMORY);
	// MOV M,C
	mov(0x71, &cpu);
	EXPECT_EQ(cpu.memory[cpu.hl], 1);
	// MOV A,M
	mov(0x7E, &cpu);
	EXPECT_EQ(cpu.psw, 0x0100);
}

TEST(MVI, ToRegister)
{
	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};
	memset(memory, 0, MAX_MEMORY);

	// MVI B
	cpu.memory[0] = 0x06;
	cpu.memory[1] = 0x01;
	mvi(0x06, &cpu);
	EXPECT_EQ(cpu.b, cpu.memory[1]);
	EXPECT_EQ(cpu.pc, 2);

	// MVI C
	cpu.memory[2] = 0x0e;
	cpu.memory[3] = 0x02;
	mvi(0x0e, &cpu);
	EXPECT_EQ(cpu.c, cpu.memory[3]);
	EXPECT_EQ(cpu.pc, 4);
	// Check that the whole register is correct.
	EXPECT_EQ(cpu.bc, 0x0102);

	// MVI D
	cpu.memory[4] = 0x16;
	cpu.memory[5] = 0x03;
	mvi(0x16, &cpu);
	EXPECT_EQ(cpu.d, cpu.memory[5]);
	EXPECT_EQ(cpu.pc, 6);

	// MVI E
	cpu.memory[6] = 0x1e;
	cpu.memory[7] = 0x04;
	mvi(0x1e, &cpu);
	EXPECT_EQ(cpu.e, cpu.memory[7]);
	EXPECT_EQ(cpu.pc, 8);
	EXPECT_EQ(cpu.de, 0x0304);

	// For HL, we'll reverse the order: write the low then the high.
	// MVI L
	cpu.memory[8] = 0x2e;
	cpu.memory[9] = 0x05;
	mvi(0x2e, &cpu);
	EXPECT_EQ(cpu.l, cpu.memory[9]);
	EXPECT_EQ(cpu.pc, 10);
	// MVI H
	cpu.memory[10] = 0x26;
	cpu.memory[11] = 0x06;
	mvi(0x26, &cpu);
	EXPECT_EQ(cpu.h, cpu.memory[11]);
	EXPECT_EQ(cpu.hl, 0x0605);
	EXPECT_EQ(cpu.pc, 12);

	// MVI A
	cpu.memory[12] = 0x3e;
	cpu.memory[13] = 0x07;
	mvi(0x3e, &cpu);
	EXPECT_EQ(cpu.a, cpu.memory[13]);
	EXPECT_EQ(cpu.pc, 14);
}

TEST(MVI, ToMem)
{
	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};
	memset(memory, 0, MAX_MEMORY);
	cpu.hl	      = 12345;
	cpu.memory[0] = 0x36;
	cpu.memory[1] = 255;
	mvi(0x36, &cpu);
	EXPECT_EQ(cpu.memory[12345], 255);
	EXPECT_EQ(cpu.pc, 2);
}

TEST(XCHG, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0x2222, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// XCHG
	xchg(0xeb, &cpu);
	EXPECT_EQ(cpu.de, 0x2222);
	EXPECT_EQ(cpu.hl, 0x1111);
	EXPECT_EQ(cpu.pc, 1);
}
