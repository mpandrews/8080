
#ifndef VBUFFER_SIZE
#	define VBUFFER_SIZE (1024 * 7)
#endif

#include "cpu.h"
#include "proms.h"
#include "taito_struct.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static inline pthread_mutex_t* create_mutex()
{
	pthread_mutex_t* new_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(new_mutex, NULL);
	return new_mutex;
}

struct taito_struct* create_taito_struct(
		struct system_resources* res, void* rstruct)
{
	struct taito_struct* new_struct = malloc(sizeof(struct taito_struct));

	pthread_mutex_t* keystate_lock	 = create_mutex();
	pthread_mutex_t* sound_lock	 = create_mutex();
	pthread_mutex_t* reset_quit_lock = create_mutex();
	pthread_mutex_t* vbuffer_lock	 = create_mutex();
	pthread_cond_t* vbuffer_cond	 = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(vbuffer_cond, NULL);
	uint8_t* vbuffer = malloc(VBUFFER_SIZE);
	memset(vbuffer, 0, VBUFFER_SIZE);

	// We use a temp struct here because the members of new_struct
	// are all const, and can't be assigned to. Nor can we use a braced
	// initializer with dynamic memory.  So we make a fully initialized temp
	// version and then do a shallow copy.
	struct taito_struct temp = {
			.vbuffer_lock	  = vbuffer_lock,
			.vbuffer_cond	  = vbuffer_cond,
			.vbuffer	  = vbuffer,
			.rom_struct	  = rstruct,
			.interrupt_buffer = res->interrupt_buffer,
			.interrupt_lock	  = res->interrupt_lock,
			.interrupt_cond	  = res->interrupt_cond,
			.keystate_lock	  = keystate_lock,
			.sound_lock	  = sound_lock,
			.reset_quit_lock  = reset_quit_lock,
			.quit_flag	  = res->quit_flag,
			.reset_flag	  = res->reset_flag,
			.proms		  = proms,
			.num_proms	  = num_proms,
	};
	memcpy(new_struct, &temp, sizeof(struct taito_struct));
	return new_struct;
}

void destroy_taito_struct(struct taito_struct* tstruct)
{
	pthread_mutex_destroy(tstruct->vbuffer_lock);
	pthread_mutex_destroy(tstruct->keystate_lock);
	pthread_mutex_destroy(tstruct->sound_lock);
	pthread_cond_destroy(tstruct->vbuffer_cond);
	free(tstruct->vbuffer_lock);
	free(tstruct->keystate_lock);
	free(tstruct->sound_lock);
	free(tstruct->vbuffer_cond);
	free(tstruct->vbuffer);
	free(tstruct);
}
