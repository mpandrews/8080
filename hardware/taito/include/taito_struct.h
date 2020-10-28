#ifndef TAITO_STRUCT
#define TAITO_STRUCT

#include <pthread.h>
#include <stdint.h>

struct taito_struct
{
	pthread_mutex_t* vbuffer_lock;
	pthread_cond_t* vbuffer_cond;
	uint8_t* vbuffer;
	void* rom_struct;
	uint8_t* interrupt_buffer;
	pthread_mutex_t* interrupt_lock;
	pthread_cond_t* interrupt_cond;
};

struct taito_struct* create_taito_struct();

void destroy_taito_struct(struct taito_struct*);

void update_vbuffer(uint8_t const*, struct taito_struct*);

#endif
