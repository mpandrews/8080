#include "rom_struct.h"

#include <SDL2/SDL.h>

int update_keystates(void* rom_struct)
{
	struct rom_struct* rStruct = (struct rom_struct*) rom_struct;
	SDL_Event e;
	int quit = 0;

	// Make sure no other thread is mutating keystates or reset/quit
	// flags at the same time.
	pthread_mutex_lock(rStruct->keystate_lock);
	pthread_mutex_lock(rStruct->reset_quit_lock);

	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			*(rStruct->quit_flag) = 1;
			quit		      = 1;
		}
		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_R: *(rStruct->reset_flag) = 1; break;
			case SDL_SCANCODE_ESCAPE:
				*(rStruct->quit_flag) = 1;
				quit		      = 1;
				break;
			case SDL_SCANCODE_C: rStruct->coin = 1; break;
			case SDL_SCANCODE_1: rStruct->p1_start = 1; break;
			case SDL_SCANCODE_2: rStruct->p2_start = 1; break;
			case SDL_SCANCODE_LEFT: rStruct->p2_left = 1; break;
			case SDL_SCANCODE_RIGHT: rStruct->p2_right = 1; break;
			case SDL_SCANCODE_UP: rStruct->p2_shoot = 1; break;
			case SDL_SCANCODE_A: rStruct->p1_left = 1; break;
			case SDL_SCANCODE_D: rStruct->p1_right = 1; break;
			case SDL_SCANCODE_W: rStruct->p1_shoot = 1; break;
			default: break;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.scancode)
			{
			// Keystate toggles - cleared on keyup
			case SDL_SCANCODE_1: rStruct->p1_start = 0; break;
			case SDL_SCANCODE_2: rStruct->p2_start = 0; break;
			case SDL_SCANCODE_LEFT: rStruct->p2_left = 0; break;
			case SDL_SCANCODE_RIGHT: rStruct->p2_right = 0; break;
			case SDL_SCANCODE_UP: rStruct->p2_shoot = 0; break;
			case SDL_SCANCODE_A: rStruct->p1_left = 0; break;
			case SDL_SCANCODE_D: rStruct->p1_right = 0; break;
			case SDL_SCANCODE_W: rStruct->p1_shoot = 0; break;
			default: break;
			}
		}
	}

	pthread_mutex_unlock(rStruct->reset_quit_lock);
	pthread_mutex_unlock(rStruct->keystate_lock);

	return quit; // return 1 if quit has been pressed
}
