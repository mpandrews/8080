
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

	unsigned char memory[(1 << 16)];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0x0001, .de = 0, .hl = 12345, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	memset(memory, 0, 1 << 16);
	// MOV M,C
	mov(0x71, &cpu);
	EXPECT_EQ(cpu.memory[cpu.hl], 1);
	// MOV A,M
	mov(0x7E, &cpu);
	EXPECT_EQ(cpu.psw, 0x0100);
}

TEST(LDAX, B_D)
{

	unsigned char memory[(1 << 16)];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0x1234, .de = 0x5678, .hl = 0, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};
	memset(memory, 0, 1 << 16);
	cpu.memory[cpu.bc] = 0xab;
	cpu.memory[cpu.de] = 0xcd;

	// LDAX B
	int cycles = ldax(0x0a, &cpu);
	EXPECT_EQ(cycles, 7);
	EXPECT_EQ(cpu.a, 0xab);

	// LDAX D
	ldax(0x1a, &cpu);
	EXPECT_EQ(cpu.a, 0xcd);
}

TEST(STAX, B_D)
{

	unsigned char memory[(1 << 16)];
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0x1234, .de = 0x5678, .hl = 0, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};
	memset(memory, 0, 1 << 16);
	cpu.a = 0xab;

	// STAX B
	int cycles = stax(0x02, &cpu);
	EXPECT_EQ(cycles, 7);
	EXPECT_EQ(cpu.memory[cpu.bc], 0xab);

	// STAX D
	stax(0x12, &cpu);
	EXPECT_EQ(cpu.memory[cpu.de], 0xab);
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
