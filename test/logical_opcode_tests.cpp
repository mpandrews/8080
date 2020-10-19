
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

TEST(ANA, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0x8222, .psw = 0x00c5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// This will clear the high four bits of cpu.a and leave the low four
	// bits unchanged resulting in 0x0d. This should also reset the sign,
	// zero, parity, and carry bits (all of which were set in the
	// cpu_state above). It should set the aux carry bit which was reset
	// in the cpu_state above.
	cpu.a = 0xfd;
	cpu.c = 0x0f;
	// ANA C
	cpu.pc += ana(0xa1, &cpu);
	EXPECT_EQ(cpu.a, 0x0d);
	EXPECT_EQ(cpu.pc, 1);
	// only the aux carry bit should be set because either (in this case
	// both) operand has bit 3 set
	EXPECT_EQ(cpu.flags, 0b00010000);
	//                     SZ-A-P-C

	// Now set register A to 0xf7 and put 0b10000001 in memory at 0x8222,
	// the address ponited to by cpu.hl. We'll call ANA M this time to
	// assert that 7 cycles instead of 4 are returned and the other ANA
	// operations also work as expected. The sign and parity flags
	// should be set after this operation. The Aux Carry flag should be
	// reset because neither 0xf7 or 0x81 sets bit 3 high.
	cpu.a		   = 0xf7;
	cpu.memory[0x8222] = 0x81;
	cpu.pc += ana(0xa6, &cpu);
	EXPECT_EQ(cpu.a, 0x81);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Lastly, we'll zero-out the C register, set the A register to 0xff,
	// AND the two, and assert that A == 0 and the zero bit is set by
	// ANA. The parity and aux carry flags should also be set by this
	// operation.
	cpu.c = 0x00;
	cpu.a = 0xff;
	cpu.pc += ana(0xa1, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, 0b01010100);
	//                     SZ-A-P-C
}

TEST(ANI, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0xff00, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// ANI
	cpu.memory[cpu.pc + 1] = 0xab;
	cpu.pc += ani(0xe6, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0xab);
	// Only sign flag is set
	EXPECT_EQ(cpu.flags, 0b10000000);

	cpu.memory[cpu.pc + 1] = 0x00;
	cpu.pc += ani(0xe6, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0x00);
	// Zero and parity flags are set
	EXPECT_EQ(cpu.flags, 0b01000100);
}

TEST(XRA, Registers_and_Memory)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0xabcd, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// XRA M
	cpu.memory[cpu.hl] = 0x12;
	cpu.pc += xra(0xae, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.a, 0x12);
	EXPECT_EQ(cpu.flags, 0b00000100);

	// XRA B
	cpu.b = 0x34;
	cpu.pc += xra(0xa8, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0x26);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA C
	cpu.c = 0x56;
	cpu.pc += xra(0xa9, &cpu);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.a, 0x70);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA D
	cpu.d = 0x78;
	cpu.pc += xra(0xaa, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0x08);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA E
	cpu.e = 0x9a;
	cpu.pc += xra(0xab, &cpu);
	EXPECT_EQ(cpu.pc, 5);
	EXPECT_EQ(cpu.a, 0x92);
	EXPECT_EQ(cpu.flags, 0b10000000);

	// XRA H
	cpu.h = 0xbc;
	cpu.pc += xra(0xac, &cpu);
	EXPECT_EQ(cpu.pc, 6);
	EXPECT_EQ(cpu.a, 0x2e);
	EXPECT_EQ(cpu.flags, 0b00000100);

	// XRA L
	cpu.l = 0xde;
	cpu.pc += xra(0xad, &cpu);
	EXPECT_EQ(cpu.pc, 7);
	EXPECT_EQ(cpu.a, 0xf0);
	EXPECT_EQ(cpu.flags, 0b10000100);

	// XRA A
	cpu.pc += xra(0xaf, &cpu);
	EXPECT_EQ(cpu.pc, 8);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, 0b01000100);
}

TEST(XRI, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0, .psw = 0xff00, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// XRI
	cpu.memory[cpu.pc + 1] = 0xab;
	cpu.pc += xri(0xee, &cpu);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.a, 0x54);
	EXPECT_EQ(cpu.flags, 0b00000000);

	cpu.memory[cpu.pc + 1] = 0xcd;
	cpu.pc += xri(0xee, &cpu);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.a, 0x99);
	// Sign and parity flags are set
	EXPECT_EQ(cpu.flags, 0b10000100);
}

TEST(ORA, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0x8222, .psw = 0x00d5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// ORA against C register. This should set all 4 low bits of the A
	// register and leave the high 4 bits unaltered. All flags shouild be
	// cleared (all were set high in the struct_cpu instantiation above),
	cpu.a = 0x23;
	cpu.c = 0x0f;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0x2f);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.flags, 0b00000000);
	//                     SZ-A-P-C

	// For this test, zero-out the A register and put 0b11101101 in
	// memory at 0x8222, the address ponited to by cpu.hl. We'll call ORA M
	// this time to assert that 7 cycles instead of 4 are returned and the
	// other ORA operations also work as expected. the sign and parity flags
	// should be set after this operation. Other flags should be reset.
	cpu.a		   = 0x00;
	cpu.memory[0x8222] = 0xed;
	cpu.pc += ora(0xb6, &cpu);
	EXPECT_EQ(cpu.a, 0xed);
	EXPECT_EQ(cpu.pc, 2);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Lastly, zero-out the A register, set the C register to 0xff, OR the
	// two and assert that A is set to 0xff
	cpu.a = 0x00;
	cpu.c = 0xff;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.pc, 3);
	EXPECT_EQ(cpu.flags, 0b10000100);
	//                     SZ-A-P-C

	// Actually, just one more test to make sure the zero flag is set
	// when it ought to be set by ORA
	cpu.a = 0x00;
	cpu.c = 0x00;
	cpu.pc += ora(0xb1, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.pc, 4);
	EXPECT_EQ(cpu.flags, 0b01000100);
	//                     SZ-A-P-C
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
	cpu.pc += stc(0x37, &cpu);
	EXPECT_EQ(cpu.pc, 1);
	EXPECT_EQ(cpu.flags, 0x00000001);

	// Call STC again and check that carry flag is still set
	stc(0x37, &cpu);
	EXPECT_EQ(cpu.flags, 0x00000001);
}
