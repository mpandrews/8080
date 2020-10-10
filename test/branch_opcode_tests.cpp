extern "C"
{
#include "cpu.h"
#include "opcode_decls.h"
}

#include "gtest/gtest.h"

/* CALL always has the same expected behavior - it takes 17 cycles, pushes
 * its argument onto the program counter, stores the next instruction
 * address in the stack, and decrements the stack pointer by 2.
 * The only complication is that there are 4 opcodes that call it */
TEST(CALL, allOpcodes)
{
	
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// argument for first call
	cpu.memory[1] = 0x10;
	cpu.memory[2] = 0x00;

	// argument for second call
	cpu.memory[0x0011] = 0x20;
	cpu.memory[0x0012] = 0x00;

	// argument for third call
	cpu.memory[0x0021] = 0x30;
	cpu.memory[0x0022] = 0x00;

	// argument for fourth call
	cpu.memory[0x0031] = 0x40;
	cpu.memory[0x0032] = 0x00;

	// call CALL, assert the return value is correct and the stack pointer
	// is as expected and the program counter is as expected
	// 0xcd
	int cycles = call(0xcd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x0010);
	EXPECT_EQ(cpu.sp, 0xfffe);

	// 0xdd
	cycles = call(0xdd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x0020);
	EXPECT_EQ(cpu.sp, 0xfffc);

	// 0xed
	cycles = call(0xed, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x0030);
	EXPECT_EQ(cpu.sp, 0xfffa);

	// 0xed
	cycles = call(0xfd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x0040);
	EXPECT_EQ(cpu.sp, 0xfff8);
}

TEST(RET, allOpcodes)
{

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// two versions of ret: 0xc9 and 0xd9
	cpu.sp = 0xfffc;
	cpu.memory[0xfffc] = 0xee;
	cpu.memory[0xfffd] = 0x00;
	cpu.memory[0xfffe] = 0xff;
	cpu.memory[0xffff] = 0x01;
	int cycles = ret(0xc9, &cpu);
	EXPECT_EQ(cycles, 10);
	EXPECT_EQ(cpu.sp, 0xfffe);
	EXPECT_EQ(cpu.pc, 0x00ee);

	cycles = ret(0xd9, &cpu);
	EXPECT_EQ(cycles, 10);
	EXPECT_EQ(cpu.sp, 0x0000);
	EXPECT_EQ(cpu.pc, 0x01ff);
}

TEST(JMP, allOpcodes)
{

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};
	// two versions of jmp: 0xc3 and 0xcb
	// set argument for first jump call
	cpu.memory[0x0001] = 0x03;
	cpu.memory[0x0002] = 0x30;
	int cycles = jmp(0xc3, &cpu);
	EXPECT_EQ(cpu.pc, 0x3003);
	EXPECT_EQ(cycles, 10);
	
	cpu.memory[0x3004] = 0x03;
	cpu.memory[0x3005] = 0x00;
	cycles = jmp(0xcb, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);
}

/* 8 JCOND opcodes jump and no jump branches for each  */
TEST(JNZ, JumpAndNoJump)
{
	uint8_t opcode = 0xc2;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory,
	// set the flag and call jnz, and
	// assert that jnz did jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory,
	// reset the flag and call jump, and
	// assert that jmp did not jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = ~flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);

}

TEST(JNC, JumpAndNoJump)
{
	// this test is for jnc, 0xd2
	uint8_t opcode = 0xd2;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond, 
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = ~flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);

}

TEST(JPO, JumpAndNoJump)
{

	// this test is for jpo, 0xe2
	uint8_t opcode = 0xe2;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	// parity flag is set when parity is even, so jpo (aka jump-parity-odd)
	// should jump when the parity flag is reset
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = ~flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);

}

TEST(JP, JumpAndNoJump)
{

	// this test is for jp, 0xf2
	uint8_t opcode = 0xf2;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = ~flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);


}

TEST(JZ, JumpAndNoJump)
{
	// this test is for jz, 0xca
	uint8_t opcode = 0xca;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = ~flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JC, JumpAndNoJump)
{

	// this test is for jc, 0xda
	uint8_t opcode = 0xda;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = ~flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);

}

TEST(JPE, JumpAndNoJump)
{
	// this test is for jpe, 0xea
	uint8_t opcode = 0xea;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = ~flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);

}

TEST(JM, JumpAndNoJump)
{

	// this test is for jm, 0xfa
	uint8_t opcode = 0xfa;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set jmp arguments in cpu memory, set the flag and call jcond,
	// and assert that jcond did not jmp
	cpu.memory[0x0001] = 0x05;
	cpu.memory[0x0002] = 0x04;
	cpu.psw = ~flag;
	int cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// set jmp arguments in cpu memory, reset the flag and call jump,
	// and assert that jmp did jmp
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;
	cpu.psw = flag;
	cycles = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

/* 8 RCOND opcodes return and no return branches for each */
TEST(RNZ, ReturnAndNoReturn)
{
	// this test is for rnz, 0xc0
	uint8_t opcode = 0xc0;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = ~flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RNC, ReturnAndNoReturn)
{
	// this test is for rnc, 0xd0
	uint8_t opcode = 0xd0;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = ~flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RPO, ReturnAndNoReturn)
{
	// this test is for rpo, 0xe0
	uint8_t opcode = 0xe0;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = ~flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RP, ReturnAndNoReturn)
{
	// this test is for rp, 0xf0
	uint8_t opcode = 0xf0;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = ~flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);

}

TEST(RZ, ReturnAndNoReturn)
{
	// this test is for rz, 0xc8
	uint8_t opcode = 0xc8;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RC, ReturnAndNoReturn)
{
	// this test is for rc, 0xd8
	uint8_t opcode = 0xd8;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RPE, ReturnAndNoReturn)
{
	// this test is for rpe, 0xe8
	uint8_t opcode = 0xe8;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RM, ReturnAndNoReturn)
{
	// this test is for rm, 0xf8
	uint8_t opcode = 0xf8;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set stack pointer and populate memory at stack pointer as if there
	// with .. something. So that return has something testable to return
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp = 0x00fe;

	// No return should take 5 cycles and simply advance the program counter
	// to the next address.
	cpu.psw = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Return case should pop 0x0504 off the stack and put it in the pc
	// and takes 11 cycles
	cpu.psw = flag;
	cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

/* 8 CCOND opcodes return and no return branches for each */
TEST(CNZ, CallAndNoCall)
{
	// this test is for cnz, 0xc4
	uint8_t opcode = 0xc4;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CNC, CallAndNoCall)
{
	// this test is for cnc, 0xd4
	uint8_t opcode = 0xd4;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CPO, CallAndNoCall)
{
	// this test is for cpo, 0xe4
	uint8_t opcode = 0xe4;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CP, CallAndNoCall)
{
	// this test is for cp, 0xf4
	uint8_t opcode = 0xf4;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CZ, CallAndNoCall)
{
	// this test is for cnz, 0xcc
	uint8_t opcode = 0xcc;
	uint8_t flag = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CC, CallAndNoCall)
{
	// this test is for cnz, 0xdc
	uint8_t opcode = 0xdc;
	uint8_t flag = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CPE, CallAndNoCall)
{
	// this test is for cpe, 0xec
	uint8_t opcode = 0xec;
	uint8_t flag = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CM, CallAndNoCall)
{
	// this test is for cm, 0xfc
	uint8_t opcode = 0xfc;
	uint8_t flag = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
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
		.bc = 0,
		.de = 0,
		.hl = 0,
		.psw = 0,
		.halt_flag = 0,
		.reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// set a call location into memory that the stack pointer will be set
	// to when the call is succesful. This will be at 0x04 and 0x05 since
	// the first call which won't be executed will advance the pc by 3
	cpu.memory[0x0004] = 0x05;
	cpu.memory[0x0005] = 0x04;

	// a no-call should take 11 cycles and simply advance the pc by 3
	cpu.psw = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);

	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x0405 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles = ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(JMP, All)
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

	// JMP <0x3412>
	cpu.memory[cpu.pc + 1] = 0x12;
	cpu.memory[cpu.pc + 2] = 0x34;
	jmp(0xc3, &cpu);
	EXPECT_EQ(cpu.pc, 0x3412);

	// JMP <0xbbaa>
	cpu.memory[cpu.pc + 1] = 0xaa;
	cpu.memory[cpu.pc + 2] = 0xbb;
	jmp(0xcb, &cpu);
	EXPECT_EQ(cpu.pc, 0xbbaa);
}
