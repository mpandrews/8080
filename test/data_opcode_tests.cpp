
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
		.int_cond = 0,
		.int_lock = 0,
		.memory = memory,
		.interrupt_buffer = 0,
		.data_bus = 0,
		.address_bus = 0,
		.sp = 0,
		.pc = 0,
		.bc = 0x0001,
		.de = 0,
		.hl = 12345,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0

	};
	memset(memory, 0, 1 << 16);
	// MOV M,C
	mov(0x71, &cpu);
	EXPECT_EQ(cpu.memory[cpu.hl], 1);
	// MOV A,M
	mov(0x7E, &cpu);
	EXPECT_EQ(cpu.psw, 0x0100);
}
