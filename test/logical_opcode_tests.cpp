
extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_size.h"
}

#include "gtest/gtest.h"

TEST(CMC, All)
{
	struct cpu_state cpu
	{
		0
	};

	cpu.flags = 0xff;
	// With all the flags set, call CMC and assert that just the Carry flag
	// was reset and all other flags were unaffected
	uint8_t opcode = 0x3f;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmc(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, 0xfe);

	// Now that the carry flag is reset, call CMC again and assert that the
	// flags register is now 0xff again
	cmc(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, 0xff);
}

TEST(CMA, All)
{
	struct cpu_state cpu
	{
		0
	};

	// cpu.a is set to 0 to start, assert that it is complemented to 0xff
	uint8_t opcode = 0x2f;
	EXPECT_EQ(get_opcode_size(opcode), 1);

	cma(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xff);

	// now that it is 0xff, complement it again and asser that it has been
	// reset to 0x00
	cpu.pc += cma(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0);
}

TEST(ANA, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
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
	uint8_t opcode = 0xa1;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	ana(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x0d);
	// only the aux carry bit should be set because either (in this case
	// both) operand has bit 3 set
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG);

	// Now set register A to 0xf7 and put 0b10000001 in memory at 0x8222,
	// the address ponited to by cpu.hl. We'll call ANA M this time to
	// assert that 7 cycles instead of 4 are returned and the other ANA
	// operations also work as expected. The sign and parity flags
	// should be set after this operation. The Aux Carry flag should be
	// reset because neither 0xf7 or 0x81 sets bit 3 high.
	opcode = 0xa6;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cpu.a		   = 0xf7;
	cpu.memory[0x8222] = 0x81;
	ana(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x81);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);

	// Lastly, we'll zero-out the C register, set the A register to 0xff,
	// AND the two, and assert that A == 0 and the zero bit is set by
	// ANA. The parity and aux carry flags should also be set by this
	// operation.
	cpu.c  = 0x00;
	cpu.a  = 0xff;
	opcode = 0xa1;
	ana(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(ANI, All)
{
	struct cpu_state cpu
	{
		0
	};
	cpu.a = 0xff;
	// ANI
	uint8_t opcode[2] = {0xe6, 0xab};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	ani(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xab);
	// Sign and AC.
	EXPECT_EQ(cpu.flags, SIGN_FLAG | AUX_CARRY_FLAG);

	opcode[1] = 0x00;
	ani(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	// Zero and parity flags are set
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
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
	uint8_t opcode = 0xae;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cpu.memory[cpu.hl] = 0x12;
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x12);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// XRA B
	cpu.b  = 0x34;
	opcode = 0xa8;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x26);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA C
	cpu.c  = 0x56;
	opcode = 0xa9;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x70);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA D
	cpu.d  = 0x78;
	opcode = 0xaa;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x08);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// XRA E
	cpu.e  = 0x9a;
	opcode = 0xab;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x92);
	EXPECT_EQ(cpu.flags, SIGN_FLAG);

	// XRA H
	cpu.h  = 0xbc;
	opcode = 0xac;
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x2e);
	EXPECT_EQ(cpu.flags, PARITY_FLAG);

	// XRA L
	cpu.l  = 0xde;
	opcode = 0xad;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xf0);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);

	// XRA A
	opcode = 0xaf;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	xra(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG);
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
	uint8_t opcode[2] = {0xee, 0xab};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	xri(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x54);
	EXPECT_EQ(cpu.flags, 0b00000000);

	opcode[1] = 0xcd;
	xri(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x99);
	// Sign and parity flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);
}

TEST(ORA, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
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
	cpu.a	       = 0x23;
	cpu.c	       = 0x0f;
	uint8_t opcode = 0xb1;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	ora(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x2f);
	EXPECT_EQ(cpu.flags, 0b00000000);

	// For this test, zero-out the A register and put 0b11101101 in
	// memory at 0x8222, the address ponited to by cpu.hl. We'll call ORA M
	// this time to assert that 7 cycles instead of 4 are returned and the
	// other ORA operations also work as expected. the sign and parity flags
	// should be set after this operation. Other flags should be reset.
	opcode = 0xb6;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cpu.a		   = 0x00;
	cpu.memory[0x8222] = 0xed;
	ora(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xed);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);

	// Lastly, zero-out the A register, set the C register to 0xff, OR the
	// two and assert that A is set to 0xff
	cpu.a  = 0x00;
	cpu.c  = 0xff;
	opcode = 0xb1;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	ora(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);

	// Actually, just one more test to make sure the zero flag is set
	// when it ought to be set by ORA
	cpu.a = 0x00;
	cpu.c = 0x00;
	ora(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x00);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG);
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
	uint8_t opcode = 0x37;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	stc(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);

	// Call STC again and check that carry flag is still set
	stc(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
}

TEST(ORI, All)
{
	struct cpu_state cpu
	{
		0
	};

	/* Set an argument in memory to OR the accumulator with. The argument
	 * 0x73 should set the accumulator to equal 0x73. Additionally, this
	 * should reset all the flags (all of which are set before the
	 * operation).
	 */
	uint8_t opcode[2] = {0xf6, 0x73};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	cpu.flags = 0b11010101; // all flags are set
	ori(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x73);
	EXPECT_EQ(cpu.flags, 0b00000000);

	/* Now OR the accumulator with 0xf0. This should leave the accumulator
	 * with the value 0xf3. This should aslo set the parity and sign flags
	 */
	opcode[1] = 0xf0;
	ori(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xf3);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG);

	/* Lastly, zero-out the accumulator and OR it with 0. This should leave
	 * the A register set to 0 and should set the Zero and Parity flags
	 */
	cpu.a	  = 0;
	opcode[1] = 0;
	ori(opcode, &cpu);
	EXPECT_EQ(cpu.a, 0);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG);
}

TEST(CPI, All)
{
	struct cpu_state cpu
	{
		0
	};

	cpu.psw = 0x02d5;
	cpu.hl	= 0x8001;

	/* The accumulator is set equal to 2 and all the flags are turned on
	 * at this point. This will compare the accumulator's value against 1.
	 * This should reset the sign, zero, aux carry, parity, and carry flags
	 */

	uint8_t opcode[2] = {0xfe, 1};
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	cpi(opcode, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG);

	/* Now we'll compare the accumulator against an equal value.
	 */
	opcode[1] = 2;
	cpi(opcode, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);

	opcode[1] = 3;
	cpi(opcode, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.flags, SIGN_FLAG | CARRY_FLAG | PARITY_FLAG);
}

TEST(CMP, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0x0,
		.hl = 0x8001, .psw = 0x00d5, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// Z flag is set to 1 if A == operand_val.
	// Carry flag s set to 1 if A < operand_val.

	// CMP B
	// compare same values, A = operand_val
	uint8_t opcode = 0xb8;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG | AUX_CARRY_FLAG);

	// CMP C
	// A < operand_val
	cpu.c  = 0x12;
	opcode = 0xb9;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// Sign, Parity, Carry flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG | CARRY_FLAG);

	// CMP D
	// A > operand_val
	cpu.a  = 0x99;
	cpu.d  = 0x11;
	opcode = 0xba;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// Sign, AC, parity flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);

	// CMP E
	cpu.a  = 0;
	cpu.e  = 0;
	opcode = 0xbb;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	EXPECT_EQ(cpu.flags, ZERO_FLAG | PARITY_FLAG | AUX_CARRY_FLAG);

	// CMP H
	// A < operand_val
	cpu.h  = 0x12;
	opcode = 0xbc;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// Sign, Parity, Carry flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | PARITY_FLAG | CARRY_FLAG);

	// CMP L
	// A > operand_val
	cpu.a  = 0x99;
	cpu.l  = 0x11;
	opcode = 0xbd;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// Sign, AC, parity flags are set
	EXPECT_EQ(cpu.flags, SIGN_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);

	// CMP M
	// A > operand_val
	cpu.a		   = 0xff;
	cpu.memory[cpu.hl] = 0xab;
	opcode		   = 0xbe;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// AC flag is set
	EXPECT_EQ(cpu.flags, AUX_CARRY_FLAG);

	// CMP A
	opcode = 0xbf;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	cmp(&opcode, &cpu);
	// Zero, AC, and parity flags are set.
	EXPECT_EQ(cpu.flags, ZERO_FLAG | AUX_CARRY_FLAG | PARITY_FLAG);
}

TEST(RLC, All)
{
	struct cpu_state cpu
	{
		0
	};

	cpu.a	       = 1;
	uint8_t opcode = 0x07;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	rlc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 2);
	EXPECT_EQ(cpu.flags, 0);

	cpu.a = 0x80;
	rlc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);

	cpu.a = 0xff;
	rlc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
}

TEST(RRC, All)
{
	struct cpu_state cpu
	{
		0
	};

	cpu.a	       = 1;
	uint8_t opcode = 0x0f;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	rrc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x80);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);

	cpu.a = 0x80;
	rrc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x40);
	EXPECT_EQ(cpu.flags, 0);

	cpu.a = 0xff;
	rrc(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xff);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
}

TEST(RAL, All)
{
	struct cpu_state cpu
	{
		0
	};

	uint8_t opcode = 0x17;
	EXPECT_EQ(get_opcode_size(opcode), 1);

	cpu.flags = CARRY_FLAG;
	ral(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 1);
	EXPECT_EQ(cpu.flags, 0);

	cpu.a = 0xff;
	ral(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xfe);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
	ral(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xfd);

	cpu.a = 0x81;
	ral(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x03);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
}

TEST(RAR, All)
{
	struct cpu_state cpu
	{
		0
	};

	uint8_t opcode = 0x1f;
	EXPECT_EQ(get_opcode_size(opcode), 1);

	cpu.flags = CARRY_FLAG;
	rar(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x80);
	EXPECT_EQ(cpu.flags, 0);

	cpu.a = 0xff;
	rar(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0x7f);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
	rar(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xbf);

	cpu.a = 0x81;
	rar(&opcode, &cpu);
	EXPECT_EQ(cpu.a, 0xc0);
	EXPECT_EQ(cpu.flags, CARRY_FLAG);
}
