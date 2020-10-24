#include "cpu.h"
#include "opcode_array.h"
#include "rom_struct.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_MEMORY_OFFSET    0x2400
#define VIDEO_MEMORY_HALF_SIZE 0xe00

extern void cycle_wait(int);

static inline void check_malloc(void* arg)
{
	if (!arg)
	{
		perror("Failure allocating memory in hardware struct "
		       "initialization");
		exit(1);
	}
}

int hw_in(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11011011);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: IN (Hardware: si)\n", cpu->pc);
#endif
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;
	uint8_t result		   = 0;

	switch (cpu->memory[cpu->pc + 1])
	{
	case 1:
		pthread_mutex_lock(rstruct->keystate_lock);
		result = rstruct->coin | rstruct->p2_start << 1
			 | rstruct->p1_start << 2 | 1 << 3
			 | rstruct->p1_shoot << 4 | rstruct->p1_left << 5
			 | rstruct->p1_right << 6 | 0 << 7;
		rstruct->coin = 0;
		pthread_mutex_unlock(rstruct->keystate_lock);
		break;
	case 2:
		pthread_mutex_lock(rstruct->keystate_lock);
		result = rstruct->dip3 | rstruct->dip5 << 1 | rstruct->tilt << 2
			 | rstruct->dip6 << 3 | rstruct->p2_shoot << 4
			 | rstruct->p2_left << 5 | rstruct->p2_right << 6
			 | rstruct->dip7 << 7;
		pthread_mutex_unlock(rstruct->keystate_lock);
		break;
	case 3:
		result = rstruct->shift_new << rstruct->shift_offset;
		result |= rstruct->shift_old >> (8 - rstruct->shift_offset);
	}

	cpu->a = result;
	cycle_wait(10);
	return 2;
}

int hw_out(uint8_t opcode, struct cpu_state* cpu)
{
	assert(opcode == 0b11010011);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "0x%4.4x: OUT (Hardware: si)\n", cpu->pc);
#endif
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;
	switch (cpu->memory[cpu->pc + 1])
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
	cycle_wait(10);
	return 2;
}

int hw_interrupt_hook(uint8_t opcode, struct cpu_state* cpu)
{
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;

	// If the interrupt is RST 8, the screen is telling us that we're at
	// the middle of the screen, so it's time to copy the top half of the
	// buffer.
	if (opcode == 0xcf)
	{
		pthread_mutex_lock(rstruct->vbuffer_lock);
		memcpy(rstruct->video_buffer,
				cpu->memory + VIDEO_MEMORY_OFFSET,
				VIDEO_MEMORY_HALF_SIZE);
		pthread_mutex_unlock(rstruct->vbuffer_lock);
		pthread_cond_signal(rstruct->vbuffer_condition);
	}
	// If the interrupt is RST 10, the screen is telling us that we've hit
	// the bottom, so it's time to copy the top half of the buffer.
	else if (opcode == 0xd7)
	{
		pthread_mutex_lock(rstruct->vbuffer_lock);
		memcpy(rstruct->video_buffer,
				cpu->memory + VIDEO_MEMORY_OFFSET
						+ VIDEO_MEMORY_HALF_SIZE,
				VIDEO_MEMORY_HALF_SIZE);
		pthread_mutex_unlock(rstruct->vbuffer_lock);
		pthread_cond_signal(rstruct->vbuffer_condition);
	}
	// Whether we updated the video buffer or not, we still execute the
	// interrupt.
	return opcodes[opcode](opcode, cpu);
}

void* hw_init_struct()
{
	struct rom_struct* rstruct = malloc(sizeof(struct rom_struct));
	rstruct->video_buffer	   = malloc(VIDEO_MEMORY_HALF_SIZE);
	check_malloc(rstruct->video_buffer);
	memset(rstruct->video_buffer, 0, VIDEO_MEMORY_HALF_SIZE);
	rstruct->vbuffer_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->video_buffer);
	pthread_mutex_init(rstruct->vbuffer_lock, NULL);
	rstruct->keystate_lock = malloc(sizeof(pthread_mutex_t));
	check_malloc(rstruct->keystate_lock);
	pthread_mutex_init(rstruct->keystate_lock, NULL);
	rstruct->vbuffer_condition = malloc(sizeof(pthread_cond_t));
	check_malloc(rstruct->vbuffer_condition);
	pthread_cond_init(rstruct->vbuffer_condition, NULL);

	return rstruct;
}

void hw_destroy_struct(void* hw_struct)
{
	struct rom_struct* rstruct = (struct rom_struct*) hw_struct;
	free(rstruct->video_buffer);
	pthread_mutex_destroy(rstruct->vbuffer_lock);
	free(rstruct->vbuffer_lock);
	pthread_mutex_destroy(rstruct->keystate_lock);
	free(rstruct->keystate_lock);
	pthread_cond_destroy(rstruct->vbuffer_condition);
	free(rstruct->vbuffer_condition);
	free(rstruct);
}
