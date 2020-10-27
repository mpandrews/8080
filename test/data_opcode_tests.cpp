
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_size.h"
}

#include "gtest/gtest.h"

TEST(MOV, RegisterToRegister)
{
	struct cpu_state cpu = {};
	cpu.bc		     = 0;
	cpu.de		     = 0xFFFF;
	cpu.hl		     = 0;

	// MOV B,D
	uint8_t opcode = 0x42;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(mov(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.bc, 0xFF00);

	// MOV H,B
	opcode = 0x60;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(mov(&opcode, &cpu), 5);
	// MOV L,B
	opcode = 0x68;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(mov(&opcode, &cpu), 5);

	EXPECT_EQ(cpu.hl, 0xFFFF);
}

TEST(MOV, RegisterToMem)
{

	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory, .bc = 0x0001, .hl = 12345
	};

	memset(memory, 0, MAX_MEMORY);
	// MOV M,C
	uint8_t opcode = 0x71;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(mov(&opcode, &cpu), 7);
	EXPECT_EQ(cpu.memory[cpu.hl], 1);
	// MOV A,M
	opcode = 0x7e;
	EXPECT_EQ(mov(&opcode, &cpu), 7);
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(cpu.psw, 0x0100);
}

TEST(MVI, ToRegister)
{
	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory
	};
	memset(memory, 0, MAX_MEMORY);

	uint8_t opcode[2] = {0x06, 0x01};
	// MVI B
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.b, 0x01);

	// MVI C
	opcode[0] = 0x0e;
	opcode[1] = 0x02;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.c, 0x02);
	// Check that the whole register is correct.
	EXPECT_EQ(cpu.bc, 0x0102);

	// MVI D
	opcode[0] = 0x16;
	opcode[1] = 0x03;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.d, 0x03);

	// MVI E
	opcode[0] = 0x1e;
	opcode[1] = 0x04;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.e, 0x04);
	EXPECT_EQ(cpu.de, 0x0304);

	// For HL, we'll reverse the order: write the low then the high.
	// MVI L
	opcode[0] = 0x2e;
	opcode[1] = 0x05;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.l, 0x05);
	// MVI H

	opcode[0] = 0x26;
	opcode[1] = 0x06;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.h, 0x06);
	EXPECT_EQ(cpu.hl, 0x0605);

	// MVI A
	opcode[0] = 0x3e;
	opcode[1] = 0x07;
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0x07);
}

TEST(MVI, ToMem)
{
	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory
	};
	memset(memory, 0, MAX_MEMORY);
	cpu.hl		  = 12345;
	uint8_t opcode[2] = {0x36, 0xff};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(mvi(opcode, &cpu), 10);
	EXPECT_EQ(cpu.memory[12345], 0xff);
}

TEST(LDA, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);

	struct cpu_state cpu
	{
		.memory = memory
	};

	uint8_t opcode[3]  = {0x3a, 0xaa, 0xbb};
	cpu.memory[0xbbaa] = 0x12;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	// LDA
	EXPECT_EQ(lda(opcode, &cpu), 13);
	EXPECT_EQ(cpu.a, 0x12);
}

TEST(STA, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);

	struct cpu_state cpu
	{
		.memory = memory
	};

	cpu.a		  = 0x12;
	uint8_t opcode[3] = {0x32, 0xaa, 0xbb};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	// STA
	EXPECT_EQ(sta(opcode, &cpu), 13);
	EXPECT_EQ(cpu.memory[0xbbaa], 0x12);
}

TEST(LHLD, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};

	uint8_t opcode[3] = {0x2a, 0xaa, 0xbb};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	*((uint16_t*) &cpu.memory[0xbbaa]) = 0x1234;

	// LHLD
	EXPECT_EQ(lhld(opcode, &cpu), 16);
	EXPECT_EQ(cpu.hl, 0x1234);
}

TEST(SHLD, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory, .hl = 0x1234
	};

	uint8_t opcode[3] = {0x22, 0xaa, 0xbb};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	// SHLD
	EXPECT_EQ(shld(opcode, &cpu), 16);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[0xbbaa]), 0x1234);
}

TEST(LDAX, B_D)
{

	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory, .bc = 0x1234, .de = 0x5678
	};
	memset(memory, 0, MAX_MEMORY);
	cpu.memory[cpu.bc] = 0xab;
	cpu.memory[cpu.de] = 0xcd;

	// LDAX B
	uint8_t opcode = 0x0a;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(ldax(&opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0xab);

	// LDAX D
	opcode = 0x1a;
	EXPECT_EQ(ldax(&opcode, &cpu), 7);
	EXPECT_EQ(cpu.a, 0xcd);
}

TEST(STAX, B_D)
{

	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory, .bc = 0x1234, .de = 0x5678
	};
	memset(memory, 0, MAX_MEMORY);
	cpu.a = 0xab;

	uint8_t opcode = 0x02;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	// STAX B
	EXPECT_EQ(stax(&opcode, &cpu), 7);
	EXPECT_EQ(cpu.memory[cpu.bc], 0xab);

	// STAX D
	opcode = 0x12;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(stax(&opcode, &cpu), 7);
	EXPECT_EQ(cpu.memory[cpu.de], 0xab);
}

TEST(XCHG, All)
{
	struct cpu_state cpu
	{
		.de = 0x1111, .hl = 0x2222
	};

	// XCHG
	uint8_t opcode = 0xeb;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xchg(&opcode, &cpu);
	EXPECT_EQ(cpu.de, 0x2222);
	EXPECT_EQ(cpu.hl, 0x1111);
}

TEST(LXI, All)
{
	unsigned char memory[MAX_MEMORY];
	struct cpu_state cpu
	{
		.memory = memory, .de = 0x1111, .hl = 0x2222
	};

	// First, set an argument of 0x8001 in memory and call lxi. The
	// program counter should be advanced to 3, the flags should be
	// unchanged, and the BC register should now contain 0x8001
	uint8_t opcode[3] = {0x01, 0x01, 0x80};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	// LXI B
	EXPECT_EQ(lxi(opcode, &cpu), 10);
	EXPECT_EQ(cpu.flags, 0);
	EXPECT_EQ(cpu.bc, 0x8001);

	// Next, set an argument of 0x0203 and call LXI D. The flags register
	// will also be set high before this operation and tested to ensure it
	// is still high afterwards
	opcode[0] = 0x11;
	opcode[1] = 0x03;
	opcode[2] = 0x02;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	cpu.flags = 0xff;
	EXPECT_EQ(lxi(opcode, &cpu), 10);
	EXPECT_EQ(cpu.flags, 0xff);
	EXPECT_EQ(cpu.de, 0x0203);
}
