extern "C"
{
#include "cpu.h"
#include "screen_timer.h"
#include "taito_struct.h"
}

#include "hw_lib_imports.h"

#include <SDL2/SDL.h>
#include <pthread.h>

#define RST2 (0xd7)
#define RST1 (0xcf)

extern "C" int foo(struct taito_struct* tStruct)
{
	SDL_Init(SDL_INIT_VIDEO);
	pthread_mutex_lock(tStruct->vbuffer_lock);
	for (;;)
	{
		// Screen populate top half of screen.
		screen_timer();

		pthread_mutex_lock(tStruct->interrupt_lock);
		if (*tStruct->interrupt_buffer)
			pthread_cond_wait(tStruct->interrupt_cond,
					tStruct->interrupt_lock);
		*tStruct->interrupt_buffer = RST1;
		pthread_mutex_unlock(tStruct->interrupt_lock);
		pthread_cond_wait(tStruct->vbuffer_cond, tStruct->vbuffer_lock);

		// Populate bottom half of screen and render.

		update_keystates(tStruct->rom_struct);
		screen_timer();

		pthread_mutex_lock(tStruct->interrupt_lock);
		if (*tStruct->interrupt_buffer)
			pthread_cond_wait(tStruct->interrupt_cond,
					tStruct->interrupt_lock);
		*tStruct->interrupt_buffer = RST2;
		pthread_mutex_unlock(tStruct->interrupt_lock);
		pthread_cond_wait(tStruct->vbuffer_cond, tStruct->vbuffer_lock);
	}
	return 0;
}
