#ifndef TAITO_STRUCT
#define TAITO_STRUCT

#include <pthread.h>
#include <stdint.h>

struct taito_struct
{
	pthread_mutex_t* const vbuffer_lock;
	pthread_cond_t* const vbuffer_cond;
	uint8_t* const vbuffer;
	void* const rom_struct;
	uint8_t* const interrupt_buffer;
	pthread_mutex_t* const interrupt_lock;
	pthread_cond_t* const interrupt_cond;
	pthread_mutex_t* const keystate_lock;
	pthread_mutex_t* const sound_lock;
	pthread_mutex_t* const reset_quit_lock;
	uint8_t* const reset_flag;
	uint8_t* const quit_flag;
};

struct taito_struct* create_taito_struct();

void destroy_taito_struct(struct taito_struct*);

#endif
