
#ifndef VBUFFER_SIZE
#	define VBUFFER_SIZE (1024 * 7)
#endif

#include "taito_struct.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct taito_struct* create_taito_struct()
{
	struct taito_struct* new_struct = malloc(sizeof(struct taito_struct));
	new_struct->vbuffer_lock	= malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(new_struct->vbuffer_lock, NULL);
	new_struct->vbuffer_cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(new_struct->vbuffer_cond, NULL);
	new_struct->vbuffer = malloc(VBUFFER_SIZE);
	//new_struct->proms = NULL;
	//new_struct->num_proms = 0;
	return new_struct;
}

void destroy_taito_struct(struct taito_struct* tstruct)
{
	pthread_mutex_destroy(tstruct->vbuffer_lock);
	pthread_cond_destroy(tstruct->vbuffer_cond);
	free(tstruct->vbuffer_lock);
	free(tstruct->vbuffer_cond);
	free(tstruct->vbuffer);
	free(tstruct);
}

void update_vbuffer(uint8_t const* source, struct taito_struct* tstruct)
{
	pthread_mutex_lock(tstruct->vbuffer_lock);
	memcpy(tstruct->vbuffer, source, VBUFFER_SIZE);
	pthread_mutex_unlock(tstruct->vbuffer_lock);
	pthread_cond_signal(tstruct->vbuffer_cond);
}
