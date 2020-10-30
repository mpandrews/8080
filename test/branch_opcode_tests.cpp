extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
#include "opcode_size.h"
}

#include "gtest/gtest.h"

/* CALL always has the same expected behavior - it takes 17 cycles, pushes
 * its argument onto the program counter, stores the next instruction
 * address in the stack, and decrements the stack pointer by 2.
 * The only complication is that there are 4 opcodes that call it */
TEST(CALL, All)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};
	cpu.sp = 0xff00;

	// Call should unconditionally perform the following actions:
	// 	1. Decrement the stack pointer by 2
	// 	2. push the next next instruction (cpu.pc + 3) onto the stack
	// 	3. set its argument (the next 2 bytes at cpu.pc + 1) as the
	// 	   program counter, and
	// 	4. take 17 clock cycles to perform
	// There are 4 opcodes that perform call 0xcd, 0xdd, 0xed, and 0xfd

	// execute CALL with opcode 0xcd
	uint8_t opcode[3] = {0xcd, 0x00, 0x80}; // 0x8000
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	EXPECT_EQ(call(opcode, &cpu), 17);
	EXPECT_EQ(cpu.pc, 0x8000);
	EXPECT_EQ(cpu.sp, 0xfefe);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0000);

	// execute CALL with opcode 0xdd
	opcode[0] = 0xdd;
	opcode[1] = 0x10;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	call(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0xfefc);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8000);

	// execute CALL with opcode 0xed
	opcode[0] = 0xed;
	opcode[1] = 0x20;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	call(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x8020);
	EXPECT_EQ(cpu.sp, 0xfefa);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8010);

	// execute CALL with opcode 0xfd
	opcode[0] = 0xfd;
	opcode[1] = 0x30;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	call(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x8030);
	EXPECT_EQ(cpu.sp, 0xfef8);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8020);
}

TEST(RET, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};

	// two versions of ret: 0xc9 and 0xd9
	cpu.sp				   = 0xfffc;
	*((uint16_t*) &cpu.memory[0xfffc]) = 0x00ee;
	*((uint16_t*) &cpu.memory[0xfffe]) = 0x01ff;

	uint8_t opcode = 0xc9;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(ret(&opcode, &cpu), 10);
	EXPECT_EQ(cpu.sp, 0xfffe);
	EXPECT_EQ(cpu.pc, 0x00ee);

	opcode = 0xd9;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	ret(&opcode, &cpu);
	EXPECT_EQ(cpu.sp, 0x0000);
	EXPECT_EQ(cpu.pc, 0x01ff);
}

TEST(JMP, All)
{

	struct cpu_state cpu = {0};

	// two versions of jmp: 0xc3 and 0xcb
	// JMP simply sets the program counter equal to its argument. JMP's
	// argument is stored in the next two bytes after JMP
	uint8_t opcode[3] = {0xc3, 0x03, 0x30};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);

	EXPECT_EQ(jmp(opcode, &cpu), 10);
	EXPECT_EQ(cpu.pc, 0x3003);

	opcode[0] = 0xcb;
	opcode[2] = 0;
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);
	jmp(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
}

/* 8 JCOND opcodes jump and no jump branches for each  */

void test_jcond(uint8_t flag, uint8_t op, uint8_t jump_if_not_flag)
{
	uint8_t opcode[3] = {op, 0x05, 0x04};
	EXPECT_EQ(get_opcode_size(opcode[0]), 3);

	struct cpu_state cpu = {0};

	// Set the cpu's zero flag and call jcond. Assert that jcond did not
	// execute a jump.
	cpu.psw = jump_if_not_flag ? flag : ~flag;
	EXPECT_EQ(jcond(opcode, &cpu), 10);
	EXPECT_EQ(cpu.pc, 0x0000);
	// Reset zero flag and set all other cpu flags. Call jcond and assert
	// that jcond did execute a jump.
	cpu.psw = ~cpu.psw;
	EXPECT_EQ(jcond(opcode, &cpu), 10);
	EXPECT_EQ(cpu.pc, 0x0405);
}

TEST(JNZ, JumpAndNoJump) { test_jcond(ZERO_FLAG, 0xc2, 1); }

TEST(JNC, JumpAndNoJump) { test_jcond(CARRY_FLAG, 0xd2, 1); }

TEST(JPO, JumpAndNoJump) { test_jcond(PARITY_FLAG, 0xe2, 1); }

TEST(JP, JumpAndNoJump) { test_jcond(SIGN_FLAG, 0xf2, 1); }

TEST(JZ, JumpAndNoJump) { test_jcond(ZERO_FLAG, 0xca, 0); }

TEST(JC, JumpAndNoJump) { test_jcond(CARRY_FLAG, 0xda, 0); }

TEST(JPE, JumpAndNoJump) { test_jcond(PARITY_FLAG, 0xea, 0); }

TEST(JM, JumpAndNoJump) { test_jcond(SIGN_FLAG, 0xfa, 0); }

void test_rcond(uint8_t flag, uint8_t opcode, uint8_t ret_if_flag_unset)
{

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};
	cpu.sp = 0xff00;

	EXPECT_EQ(get_opcode_size(opcode), 1);
	cpu.memory[0xff00] = 0x04;
	cpu.memory[0xff01] = 0x05;

	cpu.psw = ret_if_flag_unset ? flag : ~flag;
	EXPECT_EQ(retcond(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.pc, 0x0000);

	// Reset the zero flag and set all other cpu flags. Call RNZ and
	// assert that RNZ did execute a return.
	cpu.psw = ~cpu.psw;
	EXPECT_EQ(retcond(&opcode, &cpu), 11);
	EXPECT_EQ(cpu.pc, 0x0504);
}

/* 8 RCOND opcodes return and no return branches for each */
TEST(RNZ, ReturnAndNoReturn) { test_rcond(ZERO_FLAG, 0xc0, 1); }

TEST(RNC, ReturnAndNoReturn) { test_rcond(CARRY_FLAG, 0xd0, 1); }

TEST(RPO, ReturnAndNoReturn) { test_rcond(PARITY_FLAG, 0xe0, 1); }

TEST(RP, ReturnAndNoReturn) { test_rcond(SIGN_FLAG, 0xf0, 1); }

TEST(RZ, ReturnAndNoReturn) { test_rcond(ZERO_FLAG, 0xc8, 0); }

TEST(RC, ReturnAndNoReturn) { test_rcond(CARRY_FLAG, 0xd8, 0); }

TEST(RPE, ReturnAndNoReturn) { test_rcond(PARITY_FLAG, 0xe8, 0); }

TEST(RM, ReturnAndNoReturn) { test_rcond(SIGN_FLAG, 0xf8, 0); }

void test_ccond(uint8_t flag, uint8_t opc, uint8_t call_if_flag_unset)
{
	EXPECT_EQ(get_opcode_size(opc), 3);
	uint8_t opcode[3] = {opc, 0x10, 0x80};

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};
	cpu.sp = 0xff00;

	cpu.psw = call_if_flag_unset ? flag : ~flag;

	EXPECT_EQ(ccond(opcode, &cpu), 11);
	EXPECT_EQ(cpu.pc, 0);
	EXPECT_EQ(cpu.sp, 0xff00);

	cpu.psw = ~cpu.psw;
	EXPECT_EQ(ccond(opcode, &cpu), 17);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0xfefe);
}

/* 8 CCOND opcodes return and no return branches for each */
TEST(CNZ, CallAndNoCall) { test_ccond(ZERO_FLAG, 0xc4, 1); }

TEST(CNC, CallAndNoCall) { test_ccond(CARRY_FLAG, 0xd4, 1); }

TEST(CPO, CallAndNoCall) { test_ccond(PARITY_FLAG, 0xe4, 1); }

TEST(CP, CallAndNoCall) { test_ccond(SIGN_FLAG, 0xf4, 1); }

TEST(CZ, CallAndNoCall) { test_ccond(ZERO_FLAG, 0xcc, 0); }

TEST(CC, CallAndNoCall) { test_ccond(CARRY_FLAG, 0xdc, 0); }

TEST(CPE, CallAndNoCall) { test_ccond(PARITY_FLAG, 0xec, 0); }

TEST(CM, CallAndNoCall) { test_ccond(SIGN_FLAG, 0xfc, 0); }

TEST(PCHL, All)
{
	struct cpu_state cpu
	{
		0
	};
	cpu.hl = 0xabcd;

	// PCHL
	uint8_t opcode = 0xe9;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(pchl(&opcode, &cpu), 5);
	EXPECT_EQ(cpu.pc, 0xabcd);
}

TEST(RST, All)
{
	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory
	};
	cpu.sp = 0x1000;
	cpu.pc = 0xfff0;

	uint8_t opcode = 0xc7;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	EXPECT_EQ(rst(&opcode, &cpu), 11);
	// This should set the PC to 0.
	EXPECT_EQ(cpu.pc, 0);
	// It should also decrement SP by two and push PC
	// onto the stack.
	EXPECT_EQ(cpu.sp, 0x0ffe);
	EXPECT_EQ(*((uint16_t*) (cpu.memory + cpu.sp)), 0xfff0);

	// RST 0x0008
	opcode = 0xcf;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	rst(&opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0008);
	EXPECT_EQ(cpu.sp, 0x0ffc);
	EXPECT_EQ(*((uint16_t*) (cpu.memory + cpu.sp)), 0x0000);

	// RST 0x0038
	opcode = 0xff;
	EXPECT_EQ(get_opcode_size(opcode), 1);
	rst(&opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0038);
	EXPECT_EQ(cpu.sp, 0x0ffa);
	EXPECT_EQ(*((uint16_t*) (cpu.memory + cpu.sp)), 0x0008);
}
