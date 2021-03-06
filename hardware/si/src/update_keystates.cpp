#include "update_keystates.h"

#include "rom_struct.h"
#include "taito_struct.h"

void update_keystates(void* t_struct, SDL_Event* event)
{
	struct taito_struct* tStruct = (struct taito_struct*) t_struct;
	struct rom_struct* rStruct   = (struct rom_struct*) tStruct->rom_struct;

	if (event->type == SDL_KEYDOWN)
	{
		switch (event->key.keysym.scancode)
		{
		case SDL_SCANCODE_R: *(tStruct->reset_flag) = 1; break;
		case SDL_SCANCODE_C: rStruct->coin = 0; break;
		case SDL_SCANCODE_S: rStruct->p1_start = 1; break;
		case SDL_SCANCODE_DOWN: rStruct->p2_start = 1; break;
		case SDL_SCANCODE_LEFT: rStruct->p2_left = 1; break;
		case SDL_SCANCODE_RIGHT: rStruct->p2_right = 1; break;
		case SDL_SCANCODE_UP: rStruct->p2_shoot = 1; break;
		case SDL_SCANCODE_A: rStruct->p1_left = 1; break;
		case SDL_SCANCODE_D: rStruct->p1_right = 1; break;
		case SDL_SCANCODE_W: rStruct->p1_shoot = 1; break;
		case SDL_SCANCODE_T: rStruct->tilt = 1; break;
		case SDL_SCANCODE_0: rStruct->dip0 = !rStruct->dip0; break;
		case SDL_SCANCODE_1: rStruct->dip1 = !rStruct->dip1; break;
		case SDL_SCANCODE_2: rStruct->dip2 = !rStruct->dip2; break;
		case SDL_SCANCODE_3: rStruct->dip3 = !rStruct->dip3; break;
		case SDL_SCANCODE_4: rStruct->dip4 = !rStruct->dip4; break;
		case SDL_SCANCODE_5: rStruct->dip5 = !rStruct->dip5; break;
		case SDL_SCANCODE_6: rStruct->dip6 = !rStruct->dip6; break;
		case SDL_SCANCODE_7: rStruct->dip7 = !rStruct->dip7; break;
		case SDL_SCANCODE_8:
			rStruct->sound_off = !rStruct->sound_off;
			break;
		default: break;
		}
	}
	else if (event->type == SDL_KEYUP)
	{
		switch (event->key.keysym.scancode)
		{
			// Keystate toggles - cleared on keyup
		case SDL_SCANCODE_C: rStruct->coin = 1; break;
		case SDL_SCANCODE_T: rStruct->tilt = 0; break;
		case SDL_SCANCODE_S: rStruct->p1_start = 0; break;
		case SDL_SCANCODE_DOWN: rStruct->p2_start = 0; break;
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
