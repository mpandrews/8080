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
		.keystate_lock = &keystate_lock
	};
	rstruct.p1_start     = 1;
	rstruct.p2_start     = 1;
	rstruct.p2_right     = 1;
	rstruct.coin	     = 1;
	rstruct.shift_old    = 0b11001101;
	rstruct.shift_new    = 0b10101011;
	rstruct.shift_offset = 3;

	struct taito_struct tstruct = {};
	tstruct.rom_struct	    = &rstruct;

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory, .hw_struct = &tstruct
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
	EXPECT_EQ(opcodes[opcode[0]](opcode, &cpu), 10);
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

TEST(HW_OUT, Space_Invaders)
{
	pthread_mutex_t sound_lock;
	pthread_mutex_init(&sound_lock, NULL);

	struct rom_struct rstruct
	{
		.sound_lock = &sound_lock
	};
	struct taito_struct tstruct = {};
	tstruct.rom_struct	    = &rstruct;

	unsigned char memory[MAX_MEMORY];
	memset(memory, 0, MAX_MEMORY);
	struct cpu_state cpu
	{
		.memory = memory, .hw_struct = &tstruct
	};

	// Set up hw_out for Space Invaders
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
	// Assign the OUT opcode function.
	opcodes[0xd3] = (int (*)(const uint8_t*, cpu_state*)) dlsym(
			hw_lib_handle, "hw_out");
	if (!opcodes[0xd3])
	{
		fprintf(stderr,
				"Error finding function 'hw_out' in hardware"
				" library %s:\n\tlibsi.so\n",
				dlerror());
		exit(1);
	}

	// OUT port 2
	uint8_t opcode[2] = {0xd3, 2};
	cpu.a		  = 0xab;
	EXPECT_EQ(opcodes[opcode[0]](opcode, &cpu), 10);
	EXPECT_EQ(get_opcode_size(opcode[0]), 2);
	EXPECT_EQ(rstruct.shift_offset, 3);

	// OUT port 3
	opcode[1]	  = 3;
	cpu.a		  = 0b00000101;
	rstruct.ufo_sound = 1;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(rstruct.ufo_sound, 0);
	EXPECT_EQ(rstruct.shot_sound, 0);
	EXPECT_EQ(rstruct.player_die_sound, 1);
	EXPECT_EQ(rstruct.invader_killed_sound, 0);

	// OUT port 4
	opcode[1]	  = 4;
	cpu.a		  = 0b11111111;
	rstruct.shift_old = 0b11001101;
	rstruct.shift_new = 0b10101011;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(rstruct.shift_old, 0b10101011);
	EXPECT_EQ(rstruct.shift_new, 0b11111111);

	// OUT port 5
	opcode[1] = 5;
	cpu.a	  = 0b00010110;
	opcodes[opcode[0]](opcode, &cpu);
	EXPECT_EQ(rstruct.fast_invader1_sound, 0);
	EXPECT_EQ(rstruct.fast_invader2_sound, 1);
	EXPECT_EQ(rstruct.fast_invader3_sound, 1);
	EXPECT_EQ(rstruct.fast_invader4_sound, 0);
	EXPECT_EQ(rstruct.ufo_hit_sound, 1);

	pthread_mutex_destroy(&sound_lock);
}
