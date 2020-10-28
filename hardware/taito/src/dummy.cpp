extern "C"
{
#include "cpu.h"
#include "hw_lib_imports.h"
#include "screen_timer.h"
#include "taito_struct.h"
}

#include <pthread.h>

extern "C" int foo(struct taito_struct* tStruct)
{
	for (;;)
	{
		// Screen populate top half of screen.
		screen_timer();

		pthread_mutex_lock(tStruct->interrupt_lock);
		if (*tStruct->interrupt_buffer)
			pthread_cond_wait(tStruct->interrupt_cond,
					tStruct->interrupt_lock);
		*tStruct->interrupt_buffer = 0xcf;
		pthread_mutex_unlock(tStruct->interrupt_lock);

		// Populate bottom half of screen and render.

		// update_keystates needs Jen's branch merged to exist.
		// update_keystates(tStruct->rom_struct);
		screen_timer();

		pthread_mutex_lock(tStruct->interrupt_lock);
		if (*tStruct->interrupt_buffer)
			pthread_cond_wait(tStruct->interrupt_cond,
					tStruct->interrupt_lock);
		*tStruct->interrupt_buffer = 0xd7;
		pthread_mutex_unlock(tStruct->interrupt_lock);
	}
	return 0;
}
