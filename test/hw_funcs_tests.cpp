
extern "C"
{
#include "cpu.h"
#include "opcode_array.h"
#include "opcode_decls.h"
#include "opcode_size.h"
#include "si/include/rom_struct.h"
}

#include "gtest/gtest.h"
#include <dlfcn.h>

TEST(HW_IN, Space_Invaders)
{
	pthread_mutex_t keystate_lock;
	pthread_mutex_init(&keystate_lock, NULL);

	struct rom_struct rstruct
	{
		.video_buffer = nullptr, .vbuffer_lock = nullptr,
		.vbuffer_condition = nullptr, .keystate_lock = &keystate_lock,
		.p1_start = 1, .p1_shoot = 0, .p1_left = 0, .p1_right = 0,
		.p2_start = 1, .p2_shoot = 0, .p2_left = 0, .p2_right = 1,
		.tilt = 0, .dip0 = 0, .dip1 = 0, .dip2 = 0, .dip3 = 0,
		.dip4 = 0, .dip5 = 0, .dip6 = 0, .dip7 = 0, .coin = 1,
		.reset = 0, .shift_old = 0b11001101, .shift_new = 0b10101011,
		.shift_offset = 3
	};

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.int_cond = nullptr, .int_lock = nullptr, .memory = memory,
		.interrupt_buffer = nullptr, .data_bus = nullptr,
		.address_bus = nullptr, .hw_struct = &rstruct, .sp = 0, .pc = 0,
		.bc = 0, .de = 0, .hl = 0, .psw = 0, .halt_flag = 0,
		.reset_flag = 0, .interrupt_enable_flag = 0
	};

	// Set up hw_in for Space Invaders
	void* hw_lib_handle;
	hw_lib_handle = dlopen("hw/libsi.so", RTLD_NOW);
	// dlopen returns NULL on failure.
	if (!hw_lib_handle)
	{
		fprintf(stderr,
				"Error opening hardware library:\n\t%s\n",
				dlerror());
		exit(1);
	}
	// Assign the IN opcode function.
	opcodes[0xdb] = (int (*)(const uint8_t*, cpu_state*)) dlsym(
			hw_lib_handle, "hw_in");
	if (!opcodes[0xdb])
	{
		fprintf(stderr,
				"Error finding function 'hw_in' in hardware"
				" library %s:\n\tlibsi.so\n",
				dlerror());
		exit(1);
	}

	uint8_t opcode[2] = {0xdb, 1};

	// IN port 1
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(cpu.a, 0b00001111);
	EXPECT_EQ(rstruct.coin, 0); // check the coin has been cleared

	// IN port 2
	opcode[1] = 2;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(cpu.a, 0b01000000);

	// IN port 3: shift offset 3
	// 10101011 11001101
	//    rrrrr rrr
	opcode[1] = 3;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(cpu.a, 0b01011110);

	// IN port 3: shift offset 7
	// 10101011 11001101
	//        r rrrrrrr
	rstruct.shift_offset = 7;
	opcode[1]	     = 3;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(cpu.a, 0b11100110);

	pthread_mutex_destroy(&keystate_lock);
}
