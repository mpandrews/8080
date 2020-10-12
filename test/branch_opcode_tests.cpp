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
TEST(CALL, All)
{

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Call should unconditionally perform the following actions:
	// 	1. Decrement the stack pointer by 2
	// 	2. push the next next instruction (cpu.pc + 3) onto the stack
	// 	3. set its argument (the next 2 bytes at cpu.pc + 1) as the
	// 	   program counter, and
	// 	4. take 17 clock cycles to perform
	// There are 4 opcodes that perform call 0xcd, 0xdd, 0xed, and 0xfd

	// execute CALL with opcode 0xcd
	*((uint16_t*) &cpu.memory[1]) = 0x8000;
	int cycles		      = call(0xcd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x8000);
	EXPECT_EQ(cpu.sp, 0xfffe);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0003);

	// execute CALL with opcode 0xdd
	*((uint16_t*) &cpu.memory[0x8001]) = 0x8010;
	cycles				   = call(0xdd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0xfffc);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8003);

	// execute CALL with opcode 0xed
	*((uint16_t*) &cpu.memory[0x8011]) = 0x8020;
	cycles				   = call(0xed, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x8020);
	EXPECT_EQ(cpu.sp, 0xfffa);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8013);

	// execute CALL with opcode 0xfd
	*((uint16_t*) &cpu.memory[0x8021]) = 0x8030;
	cycles				   = call(0xfd, &cpu);
	EXPECT_EQ(cycles, 17);
	EXPECT_EQ(cpu.pc, 0x8030);
	EXPECT_EQ(cpu.sp, 0xfff8);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x8023);
}

TEST(RET, All)
{
	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// two versions of ret: 0xc9 and 0xd9
	cpu.sp				   = 0xfffc;
	*((uint16_t*) &cpu.memory[0xfffc]) = 0x00ee;
	*((uint16_t*) &cpu.memory[0xfffe]) = 0x01ff;

	int cycles = ret(0xc9, &cpu);
	EXPECT_EQ(cycles, 10);
	EXPECT_EQ(cpu.sp, 0xfffe);
	EXPECT_EQ(cpu.pc, 0x00ee);

	cycles = ret(0xd9, &cpu);
	EXPECT_EQ(cycles, 10);
	EXPECT_EQ(cpu.sp, 0x0000);
	EXPECT_EQ(cpu.pc, 0x01ff);
}

TEST(JMP, All)
{

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// two versions of jmp: 0xc3 and 0xcb
	// JMP simply sets the program counter equal to its argument. JMP's
	// argument is stored in the next two bytes after JMP
	*((uint16_t*) &cpu.memory[0x0001]) = 0x3003;
	int cycles			   = jmp(0xc3, &cpu);
	EXPECT_EQ(cpu.pc, 0x3003);
	EXPECT_EQ(cycles, 10);

	*((uint16_t*) &cpu.memory[0x3004]) = 0x0003;
	cycles				   = jmp(0xcb, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);
}

/* 8 JCOND opcodes jump and no jump branches for each  */
TEST(JNZ, JumpAndNoJump)
{
	// JNZ = "jump not zero" -- Jump when zero flag is reset.
	uint8_t opcode = 0xc2;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set argument address for JNZ in cpu.memory at cpu.pc+1, set the
	// cpu's zero flag and call JNZ. Assert that JNZ did not execute
	// a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set the argument address for JNZ in cpu.memory at cpu.pc+1, reset
	// zero flag and set all other cpu flags. Call JNZ and assert that
	// JNZ did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = ~flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JNC, JumpAndNoJump)
{
	// JNC = "Jjmp no carry" -- Jump when carry flag is reset.
	uint8_t opcode = 0xd2;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set argument address for JNC in cpu.memory at cpu.pc+1, set the
	// cpu's carry flag and call JNC. Assert that JNC did not execute
	// a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set the argument address for JNC in cpu.memory at cpu.pc+1, reset
	// carry flag and set all other cpu flags. Call JNC and assert that
	// JNC did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = ~flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JPO, JumpAndNoJump)
{
	// JPO = "jump parity odd" -- Jump when parity flag is reset.
	uint8_t opcode = 0xe2;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set argument address for JPO in cpu.memory at cpu.pc+1, set the
	// cpu's parity flag and call JPO. Assert that JPO did not execute
	// a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set the argument address for JPO in cpu.memory at cpu.pc+1, reset
	// parity flag and set all other cpu flags. Call JPO and assert that
	// JPO did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = ~flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JP, JumpAndNoJump)
{
	// JP = "jump positive" -- Jump when sign flag is reset.
	uint8_t opcode = 0xf2;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set argument address for JP in cpu.memory at cpu.pc+1, set the
	// cpu's sign flag and call JP. Assert that JP did not execute
	// a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set the argument address for JP in cpu.memory at cpu.pc+1, reset
	// sign flag and set all other cpu flags. Call JP and assert that
	// JP did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = ~flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JZ, JumpAndNoJump)
{
	// JZ = "Jump zero" -- Jump when zero flag is set.
	uint8_t opcode = 0xca;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set the argument address for JZ in cpu.memory at cpu.pc+1, reset
	// the zero flag and set all other cpu flags. Call JZ and assert
	// that JZ did not execute a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = ~flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set argument address for JZ in cpu.memory at cpu.pc+1, set the
	// cpu's zero flag, reset all other flags, and call JZ. Assert
	// that JZ did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JC, JumpAndNoJump)
{

	// JC = "Jump carry" -- Jump when carry flag is set.
	uint8_t opcode = 0xda;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set the argument address for JC in cpu.memory at cpu.pc+1, reset
	// the carry flag and set all other cpu flags. Call JC and assert
	// that JC did not execute a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = ~flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set argument address for JC in cpu.memory at cpu.pc+1, set the
	// cpu's carry flag, reset all other flags, and call JC. Assert
	// that JC did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JPE, JumpAndNoJump)
{
	// JPE = "Jump parity even" -- Jump when parity flag is set.
	uint8_t opcode = 0xea;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set the argument address for JPE in cpu.memory at cpu.pc+1, reset
	// the parity flag and set all other cpu flags. Call JPE and assert
	// that JPE did not execute a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = ~flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set argument address for JPE in cpu.memory at cpu.pc+1, set the
	// cpu's parity flag, reset all other flags, and call JPE. Assert
	// that JPE did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

TEST(JM, JumpAndNoJump)
{

	// JM = "Jump minus" -- Jump when sign flag is set.
	// this test is for jm, 0xfa
	uint8_t opcode = 0xfa;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set the argument address for JM in cpu.memory at cpu.pc+1, reset
	// the sign flag and set all other cpu flags. Call JM and assert
	// that JM did not execute a jump.
	*((uint16_t*) &cpu.memory[0x0001]) = 0x0405;
	cpu.psw				   = ~flag;
	int cycles			   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 10);

	// Set argument address for JM in cpu.memory at cpu.pc+1, set the
	// cpu's sign flag, reset all other flags, and call JM. Assert
	// that JM did execute a jump.
	*((uint16_t*) &cpu.memory[0x0004]) = 0x0405;
	cpu.psw				   = flag;
	cycles				   = jcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 10);
}

/* 8 RCOND opcodes return and no return branches for each */
TEST(RNZ, ReturnAndNoReturn)
{
	// RNZ = "return not zero" -- Return when zero flag is reset.
	uint8_t opcode = 0xc0;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp		   = 0x00fe;

	// Set the zero flag and reset all other flags. Call RNZ and assert
	// that RNZ did not execute a return.
	cpu.psw	   = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Reset the zero flag and set all other cpu flags. Call RNZ and
	// assert that RNZ did execute a return.
	cpu.psw = ~flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RNC, ReturnAndNoReturn)
{
	// RNC = "return no carry"-- Return when carry flag is reset.
	uint8_t opcode = 0xd0;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp		   = 0x00fe;

	// Set the carry flag and reset all other flags. Call RNC and assert
	// that RNC did not execute a return.
	cpu.psw	   = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Reset the carry flag and set all other cpu flags. Call RNC and
	// assert that RNC did execute a return.
	cpu.psw = ~flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RPO, ReturnAndNoReturn)
{
	// RPO = "return parity odd" -- Return when parity flag is reset.
	uint8_t opcode = 0xe0;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp		   = 0x00fe;

	// Set the parity flag and reset all other flags. Call RPO and assert
	// that RPO did not execute a return.
	cpu.psw	   = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Reset the parity flag and set all other cpu flags. Call RPO and
	// assert that RPO did execute a return.
	cpu.psw = ~flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RP, ReturnAndNoReturn)
{
	// RP = "return positive" -- Return when sign flag is reset.
	uint8_t opcode = 0xf0;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp		   = 0x00fe;

	// Set the sign flag and reset all other flags. Call RP and assert
	// that RP did not execute a return.
	cpu.psw	   = flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Reset the sign flag and set all other cpu flags. Call RP and
	// assert that RP did execute a return.
	cpu.psw = ~flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RZ, ReturnAndNoReturn)
{
	// RZ = "return zero" -- Return when zero flag is set.
	uint8_t opcode = 0xc8;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	cpu.memory[0x00ff] = 0x05;
	cpu.memory[0x00fe] = 0x04;
	cpu.sp		   = 0x00fe;

	// Reset the zero flag and set all other flags. Call RZ and assert
	// that RZ did not execute a return.
	cpu.psw	   = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Set the zero flag and reset all other cpu flags. Call RZ and
	// assert that RZ did execute a return.
	cpu.psw = flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0504);
	EXPECT_EQ(cycles, 11);
}

TEST(RC, ReturnAndNoReturn)
{
	// RC = "return carry" -- Return when carry flag is set.
	// this test is for rc, 0xd8
	uint8_t opcode = 0xd8;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	*((uint16_t*) &cpu.memory[0x00fe]) = 0x0405;
	cpu.sp				   = 0x00fe;

	// Reset the carry flag and set all other flags. Call RC and assert
	// that RC did not execute a return.
	cpu.psw	   = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Set the carry flag and reset all other cpu flags. Call RC and
	// assert that RC did execute a return.
	cpu.psw = flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 11);
}

TEST(RPE, ReturnAndNoReturn)
{
	// RPE = "return parity odd" -- Return when parity flag is set.
	// this test is for rpe, 0xe8
	uint8_t opcode = 0xe8;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	*((uint16_t*) &cpu.memory[0x00fe]) = 0x0405;
	cpu.sp				   = 0x00fe;

	// Reset the parity flag and set all other flags. Call RPE and assert
	// that RPE did not execute a return.
	cpu.psw	   = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Set the parity flag and reset all other cpu flags. Call RPE  and
	// assert that RPE did execute a return.
	cpu.psw = flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 11);
}

TEST(RM, ReturnAndNoReturn)
{
	// RM = "return minus" -- Return when sign flag is set.
	// this test is for rm, 0xf8
	uint8_t opcode = 0xf8;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// set stack pointer and put something in memory on the stack
	*((uint16_t*) &cpu.memory[0x00fe]) = 0x0405;
	cpu.sp				   = 0x00fe;

	// Reset the sign flag and set all other flags. Call RM and assert
	// that RM did not execute a return.
	cpu.psw	   = ~flag;
	int cycles = retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0001);
	EXPECT_EQ(cycles, 5);

	// Set the sign flag and reset all other cpu flags. Call RM  and
	// assert that RM did execute a return.
	cpu.psw = flag;
	cycles	= retcond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0405);
	EXPECT_EQ(cycles, 11);
}

/* 8 CCOND opcodes return and no return branches for each */
TEST(CNZ, CallAndNoCall)
{
	// CNZ = "Call not zero" -- execute call if zero flag is not set
	uint8_t opcode = 0xc4;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CNZ into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Set the zero flag and reset all other flags. Call CNZ and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Reset the zero flag and set all the ther flags and call CNZ. Assert
	// that CNZ was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CNC, CallAndNoCall)
{
	// CNC = "Call no carry" -- execute call if carry flag is not set
	uint8_t opcode = 0xd4;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CNC into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Set the carry flag and reset all other flags. Call CNC and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Reset the carry flag and set all the ther flags and call CNC. Assert
	// that CNC was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CPO, CallAndNoCall)
{
	// CPO = "Call parity odd" -- execute call if parity flag is not set
	uint8_t opcode = 0xe4;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CPO into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Set the parity flag and reset all other flags. Call CPO and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Reset the parity flag and set all the ther flags and call CPO. Assert
	// that CPO was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CP, CallAndNoCall)
{
	// CP = "Call positive" -- execute call if sign flag is not set
	uint8_t opcode = 0xf4;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CP into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Set the sign flag and reset all other flags. Call CP and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Reset the sign flag and set all the ther flags and call CP. Assert
	// that CP was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = ~flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CZ, CallAndNoCall)
{
	// CZ = "Call zero" -- execute call if zero flag is set
	uint8_t opcode = 0xcc;
	uint8_t flag   = ZERO_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CZ into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Reset the zero flag and set all other flags. Call CZ and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Set the zero flag and reset all the ther flags and call CZ. Assert
	// that CZ was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CC, CallAndNoCall)
{
	// CC = "Call carry" -- execute call if carry flag is set
	uint8_t opcode = 0xdc;
	uint8_t flag   = CARRY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CC into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Reset the carry flag and set all other flags. Call CC and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Set the carry flag and reset all the ther flags and call CC. Assert
	// that CC was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CPE, CallAndNoCall)
{
	// CPE = "Call parity even" -- execute call if parity flag is set
	uint8_t opcode = 0xec;
	uint8_t flag   = PARITY_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CPE into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Reset the parity flag and set all other flags. Call CPE and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Set the parity flag and reset all the ther flags and call CPE. Assert
	// that CPE was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(CM, CallAndNoCall)
{
	// CM = "Call minus" -- execute call if sign flag is set
	uint8_t opcode = 0xfc;
	uint8_t flag   = SIGN_FLAG;

	unsigned char memory[(1 << 16)];
	memset(memory, 0, 1 << 16);
	struct cpu_state cpu
	{
		.int_cond = 0, .int_lock = 0, .memory = memory,
		.interrupt_buffer = 0, .data_bus = 0, .address_bus = 0, .sp = 0,
		.pc = 0, .bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Put an argument for the second call to CM into memory at 0x0004.
	// The first call which should not be executed will advance the PC to
	// 0x0003
	*((uint16_t*) &cpu.memory[0x0004]) = 0x8010;

	// Reset the sign flag and set all other flags. Call CM and assert
	// that it advanced the program counter by 3 and nothing else.
	cpu.psw	   = ~flag;
	int cycles = ccond(opcode, &cpu);
	EXPECT_EQ(cpu.pc, 0x0003);
	EXPECT_EQ(cycles, 11);
	EXPECT_EQ(cpu.sp, 0);

	// Set the sign flag and reset all the ther flags and call CM. Assert
	// that CM was executed by checking the following:
	// Successful call should:
	// 	1. place 0x0006 onto the stack
	// 	2. move the pc to 0x8010 (the argument given to call),
	// 	3. decrement the sp by 2,
	// 	4. take 17 cycles
	cpu.psw = flag;
	cycles	= ccond(opcode, &cpu);
	EXPECT_EQ(*((uint16_t*) &cpu.memory[cpu.sp]), 0x0006);
	EXPECT_EQ(cpu.pc, 0x8010);
	EXPECT_EQ(cpu.sp, 0Xfffe);
	EXPECT_EQ(cycles, 17);
}

TEST(JMP, AllOpcodes)
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

	// JMP <0x1234>
	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0x1234;
	jmp(0xc3, &cpu);
	EXPECT_EQ(cpu.pc, 0x1234);

	// JMP <0xbbaa>
	*((uint16_t*) &cpu.memory[cpu.pc + 1]) = 0xbbaa;
	jmp(0xcb, &cpu);
	EXPECT_EQ(cpu.pc, 0xbbaa);
}

TEST(PCHL, All)
{
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = nullptr,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .sp = 0, .pc = 0, .bc = 0, .de = 0,
		.hl = 0xabcd, .psw = 0, .halt_flag = 0, .reset_flag = 0,
		.interrupt_enable_flag = 0
	};

	// PCHL
	int cycles = pchl(0xe9, &cpu);
	EXPECT_EQ(cycles, 5);
	EXPECT_EQ(cpu.pc, 0xabcd);
}
