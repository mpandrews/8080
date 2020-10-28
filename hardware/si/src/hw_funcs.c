#include "cpu.h"
#include "opcode_array.h"
#include "rom_struct.h"
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

void* front_end(void* arg)
{
	(void) arg;
#ifdef VERBOSE
	fprintf(stderr, "SI FRONT END THREAD RUNNING!\n");
#endif

	return NULL;
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
		// JEN
		break;
	case 4:
		rstruct->shift_old = rstruct->shift_new;
		rstruct->shift_new = cpu->a;
		break;
	case 5:
		// JEN
		break;
	case 6:
		// JEN
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
	// Allocate memory for the ROM struct, which lives inside the taito
	// struct.
	struct rom_struct* rstruct = malloc(sizeof(struct rom_struct));
	check_malloc(rstruct);
	rstruct->keystate_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->keystate_lock);
	pthread_mutex_init(rstruct->keystate_lock, NULL);
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
	free(rstruct->keystate_lock);
	free(rstruct);
	destroy_taito_struct(tstruct);
}
