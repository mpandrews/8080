
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
	cpu.pc += mvi(0x06, &cpu);
	EXPECT_EQ(cpu.b, cpu.memory[1]);
	EXPECT_EQ(cpu.pc, 2);

	// MVI C
	cpu.memory[2] = 0x0e;
	cpu.memory[3] = 0x02;
	cpu.pc += mvi(0x0e, &cpu);
	EXPECT_EQ(cpu.c, cpu.memory[3]);
	EXPECT_EQ(cpu.pc, 4);
	// Check that the whole register is correct.
	EXPECT_EQ(cpu.bc, 0x0102);

	// MVI D
	cpu.memory[4] = 0x16;
	cpu.memory[5] = 0x03;
	cpu.pc += mvi(0x16, &cpu);
	EXPECT_EQ(cpu.d, cpu.memory[5]);
	EXPECT_EQ(cpu.pc, 6);

	// MVI E
	cpu.memory[6] = 0x1e;
	cpu.memory[7] = 0x04;
	cpu.pc += mvi(0x1e, &cpu);
	EXPECT_EQ(cpu.e, cpu.memory[7]);
	EXPECT_EQ(cpu.pc, 8);
	EXPECT_EQ(cpu.de, 0x0304);

	// For HL, we'll reverse the order: write the low then the high.
	// MVI L
	cpu.memory[8] = 0x2e;
	cpu.memory[9] = 0x05;
	cpu.pc += mvi(0x2e, &cpu);
	EXPECT_EQ(cpu.l, cpu.memory[9]);
	EXPECT_EQ(cpu.pc, 10);
	// MVI H
	cpu.memory[10] = 0x26;
	cpu.memory[11] = 0x06;
	cpu.pc += mvi(0x26, &cpu);
	EXPECT_EQ(cpu.h, cpu.memory[11]);
	EXPECT_EQ(cpu.hl, 0x0605);
	EXPECT_EQ(cpu.pc, 12);

	// MVI A
	cpu.memory[12] = 0x3e;
	cpu.memory[13] = 0x07;
	cpu.pc += mvi(0x3e, &cpu);
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
	cpu.pc += mvi(0x36, &cpu);
	EXPECT_EQ(cpu.memory[12345], 255);
	EXPECT_EQ(cpu.pc, 2);
}

TEST(LDA, All)
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

	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0xbbaa;
	cpu.memory[0xbbaa]		       = 0x12;

	// LDA
	cpu.pc += lda(0x3a, &cpu);
	EXPECT_EQ(cpu.a, 0x12);
	EXPECT_EQ(cpu.pc, 3);
}

TEST(STA, All)
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

	cpu.a				       = 0x12;
	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0xbbaa;

	// STA
	cpu.pc += sta(0x32, &cpu);
	EXPECT_EQ(cpu.memory[0xbbaa], 0x12);
	EXPECT_EQ(cpu.pc, 3);
}

TEST(LHLD, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0xbbaa;
	*((uint16_t*) &cpu.memory[0xbbaa])     = 0x1234;

	// LHLD
	cpu.pc += lhld(0x2a, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.hl, 0x1234);
}

TEST(SHLD, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0x1234, .psw = 0,
		.halt_flag = 0, .reset_flag = 0, .interrupt_enable_flag = 0
	};

	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0xbbaa;

	// SHLD
	cpu.pc += shld(0x22, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[0xbbaa]), 0x1234);
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
	cpu.pc += ldax(0x0a, &cpu);
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
	cpu.pc += stax(0x02, &cpu);
	EXPECT_EQ(cpu.memory[cpu.bc], 0xab);

	// STAX D
	cpu.pc += stax(0x12, &cpu);
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
	cpu.pc += xchg(0xeb, &cpu);
	EXPECT_EQ(cpu.de, 0x2222);
	EXPECT_EQ(cpu.hl, 0x1111);
	EXPECT_EQ(cpu.pc, 1);
}

TEST(LXI, All)
{
	unsigned char memory[(1 << 16)];
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x1111,
		.hl = 0x2222, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// First, set an argument of 0x8001 in memory and call lxi. The
	// program counter should be advanced to 3, the flags should be
	// unchanged, and the BC register should now contain 0x8001
	// LXI B
	*((uint16_t*) &cpu.memory[1]) = 0x8001;
	cpu.pc += lxi(0x01, &cpu);
	EXPECT_EQ(cpu.flags, 0);
	EXPECT_EQ(cpu.bc, 0x8001);
	EXPECT_EQ(cpu.pc, 3);

	// Next, set an argument of 0x0203 and call LXI D. The flags register
	// will also be set high before this operation and tested to ensure it
	// is still high afterwards
	*((uint16_t*) &cpu.memory[4]) = 0x0203;
	cpu.flags		      = 0xff;
	cpu.pc += lxi(0x11, &cpu);
	EXPECT_EQ(cpu.flags, 0xff);
	EXPECT_EQ(cpu.de, 0x0203);
	EXPECT_EQ(cpu.pc, 6);
}
