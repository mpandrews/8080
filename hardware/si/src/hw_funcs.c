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

int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) opcode;

#ifdef VERBOSE
	fprintf(stderr, "IN 0x%2.2x (Hardware: si)\n", opcode[1]);
#endif
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;
	switch (opcode[1])
	{
	case 0:
		// JEN
		break;
	case 1:
		// JEN
		break;
	case 2:
		// JEN
		break;
	case 3:
		cpu->a = rstruct->shift_new << rstruct->shift_offset;
		cpu->a |= rstruct->shift_old >> (8 - rstruct->shift_offset);
	}
	cycle_wait(10);
	return 2;
}

int hw_out(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11010011);
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "OUT 0x%2.2x (Hardware: si)\n", opcode[1]);
#endif
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;
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
	cycle_wait(10);
	return 2;
}

int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*))
{
	struct rom_struct* rstruct = (struct rom_struct*) cpu->hw_struct;

	// If the interrupt is RST 8, the screen is telling us that we're at
	// the middle of the screen, so it's time to copy the top half of the
	// buffer.
	if (opcode[0] == 0xcf)
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
	else if (opcode[0] == 0xd7)
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
#ifdef VERBOSE
	fprintf(stderr, "INT   : ");
#endif
	return op_func(opcode, cpu);
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
