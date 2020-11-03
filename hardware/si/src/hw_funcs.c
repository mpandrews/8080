#include "cpu.h"
#include "opcode_array.h"
#include "rom_struct.h"
#include "taito_start.h"
#include "taito_struct.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_MEMORY_OFFSET    0x2400
#define VIDEO_MEMORY_HALF_SIZE 0xe00

static inline void check_malloc(void* arg)
{
	if (!arg)
	{
		perror("Failure allocating memory in hardware struct "
		       "initialization");
		exit(1);
	}
}

int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "IN 0x%2.2x (Hardware: si)\n", opcode[1]);
#endif
	struct taito_struct* tstruct = (struct taito_struct*) cpu->hw_struct;
	struct rom_struct* rstruct   = (struct rom_struct*) tstruct->rom_struct;

	switch (opcode[1])
	{
	case 1:
		pthread_mutex_lock(rstruct->keystate_lock);
		cpu->a = rstruct->coin | rstruct->p2_start << 1
			 | rstruct->p1_start << 2 | 1 << 3
			 | rstruct->p1_shoot << 4 | rstruct->p1_left << 5
			 | rstruct->p1_right << 6 | 0 << 7;
		rstruct->coin = 0;
		pthread_mutex_unlock(rstruct->keystate_lock);
		break;
	case 2:
		pthread_mutex_lock(rstruct->keystate_lock);
		cpu->a = rstruct->dip3 | rstruct->dip5 << 1 | rstruct->tilt << 2
			 | rstruct->dip6 << 3 | rstruct->p2_shoot << 4
			 | rstruct->p2_left << 5 | rstruct->p2_right << 6
			 | rstruct->dip7 << 7;
		rstruct->tilt = 0;
		pthread_mutex_unlock(rstruct->keystate_lock);
		break;
	case 3:
		cpu->a = rstruct->shift_new << rstruct->shift_offset;
		cpu->a |= rstruct->shift_old >> (8 - rstruct->shift_offset);
	}

	return 10;
}

int hw_out(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11010011);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "OUT 0x%2.2x (Hardware: si)\n", opcode[1]);
#endif
	struct taito_struct* tstruct = (struct taito_struct*) cpu->hw_struct;
	struct rom_struct* rstruct   = (struct rom_struct*) tstruct->rom_struct;
	switch (opcode[1])
	{
	case 2: rstruct->shift_offset = cpu->a & 0x07; break;
	case 3:
		pthread_mutex_lock(rstruct->sound_lock);
		rstruct->ufo_sound = cpu->a & 1;
		if (cpu->a & (1 << 1)) { rstruct->shot_sound = 1; }
		if (cpu->a & (1 << 2)) { rstruct->player_die_sound = 1; }
		if (cpu->a & (1 << 3)) { rstruct->invader_killed_sound = 1; }
		pthread_mutex_unlock(rstruct->sound_lock);
		break;
	case 4:
		rstruct->shift_old = rstruct->shift_new;
		rstruct->shift_new = cpu->a;
		break;
	case 5:
		pthread_mutex_lock(rstruct->sound_lock);
		if (cpu->a & 1) { rstruct->fast_invader1_sound = 1; }
		if (cpu->a & (1 << 1)) { rstruct->fast_invader2_sound = 1; }
		if (cpu->a & (1 << 2)) { rstruct->fast_invader3_sound = 1; }
		if (cpu->a & (1 << 3)) { rstruct->fast_invader4_sound = 1; }
		if (cpu->a & (1 << 4)) { rstruct->ufo_hit_sound = 1; }
		pthread_mutex_unlock(rstruct->sound_lock);
		if (cpu->a & (1 << 5))
		{
			// Cocktail mode control; flip screen
		}
		break;
	}
	return 10;
}

int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*))
{
	struct taito_struct* tstruct = (struct taito_struct*) cpu->hw_struct;

	// If the interrupt is RST 8, the screen is telling us that we're at
	// the middle of the screen, so it's time to copy the top half of the
	// buffer.
	if (opcode[0] == 0xcf || opcode[0] == 0xd7)
		update_vbuffer(cpu->memory + VIDEO_MEMORY_OFFSET, tstruct);
	// Whether we updated the video buffer or not, we still execute the
	// interrupt.
	return op_func(opcode, cpu);
}

void* hw_init_struct(struct system_resources* res)
{
	// Call the constructor for the taito library's struct.
	struct taito_struct* tstruct = create_taito_struct();
	check_malloc(tstruct);
	tstruct->interrupt_buffer = res->interrupt_buffer;
	tstruct->interrupt_lock	  = res->interrupt_lock;
	tstruct->interrupt_cond	  = res->interrupt_cond;
	// Allocate memory for the ROM struct, which lives inside the taito
	// struct.
	struct rom_struct* rstruct = malloc(sizeof(struct rom_struct));
	check_malloc(rstruct);
	memset(rstruct, 0, sizeof(struct rom_struct));
	rstruct->keystate_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->keystate_lock);
	pthread_mutex_init(rstruct->keystate_lock, NULL);
	rstruct->dip0 = 1;
	rstruct->dip1 = 1;
	rstruct->dip2 = 1;
	rstruct->dip3 = 1;
	rstruct->dip4 = 1;
	rstruct->dip5 = 1;
	rstruct->dip6 = 1;
	rstruct->dip7 = 1;

	rstruct->sound_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->sound_lock);
	rstruct->sound_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->sound_lock);
	pthread_mutex_init(rstruct->sound_lock, NULL);
	// Assign the pointers to the shared CPU struct resources.
	rstruct->reset_quit_lock = res->reset_quit_lock;
	rstruct->reset_flag	 = res->reset_flag;
	rstruct->quit_flag	 = res->quit_flag;
	// Assign our rom struct into the taito struct.
	tstruct->rom_struct = rstruct;

	return tstruct;
}

void hw_destroy_struct(void* hw_struct)
{
	struct taito_struct* tstruct = (struct taito_struct*) hw_struct;
	struct rom_struct* rstruct   = (struct rom_struct*) tstruct->rom_struct;
	pthread_mutex_destroy(rstruct->keystate_lock);
	pthread_mutex_destroy(rstruct->sound_lock);
	free(rstruct->keystate_lock);
	free(rstruct->sound_lock);
	free(rstruct);
	destroy_taito_struct(tstruct);
}

void* front_end(void* tstruct)
{
	taito_start((struct taito_struct*) tstruct);
	return NULL;
}
