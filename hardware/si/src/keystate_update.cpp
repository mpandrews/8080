#include "rom_struct.h"

#include <SDL2/SDL.h>
#include <iostream>

extern "C"
{
	void keystate_update(struct rom_struct* rStruct)
	{
		SDL_Event e;
		bool quit = false;

		while (!quit && SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				std::cout << "Quit game" << std::endl;
				quit = true;
			}

			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_R:
					std::cout << "RESET pressed"
						  << std::endl;
					rStruct->reset = 1;
					break;
				case SDL_SCANCODE_ESCAPE:
					std::cout << "QUIT" << std::endl;
					quit = true;
					break;
				case SDL_SCANCODE_C:
					std::cout << "COIN" << std::endl;
					rStruct->coin = 1;
					break;
				case SDL_SCANCODE_1:
					std::cout << "Player 1 Start"
						  << std::endl;
					rStruct->p1_start = 1;
					break;
				case SDL_SCANCODE_2:
					std::cout << "Player 2 Start"
						  << std::endl;
					rStruct->p2_start = 1;
					break;
				case SDL_SCANCODE_LEFT:
					std::cout << "P2 LEFT" << std::endl;
					rStruct->p2_left = 1;
					break;
				case SDL_SCANCODE_RIGHT:
					std::cout << "P2 RIGHT " << std::endl;
					rStruct->p2_right = 1;
					break;
				case SDL_SCANCODE_UP:
					std::cout << "P2 SHOOT" << std::endl;
					rStruct->p2_shoot = 1;
					break;
				case SDL_SCANCODE_A:
					std::cout << "P1 LEFT" << std::endl;
					rStruct->p1_left = 1;
					break;
				case SDL_SCANCODE_D:
					std::cout << "P1 RIGHT" << std::endl;
					rStruct->p1_right = 1;
					break;
				case SDL_SCANCODE_W:
					std::cout << "P1 SHOOT" << std::endl;
					rStruct->p1_shoot = 1;
					break;
				default: break;
				}
			}
			if (e.type == SDL_KEYUP)
			{
				switch (e.key.keysym.scancode)
				{
				// Keystate toggles - cleared on keyup
				case SDL_SCANCODE_1:
					rStruct->p1_start = 0;
					break;
				case SDL_SCANCODE_2:
					rStruct->p2_start = 0;
					break;
				case SDL_SCANCODE_LEFT:
					rStruct->p2_left = 0;
					break;
				case SDL_SCANCODE_RIGHT:
					rStruct->p2_right = 0;
					break;
				case SDL_SCANCODE_UP:
					rStruct->p2_shoot = 0;
					break;
				case SDL_SCANCODE_A:
					rStruct->p1_left = 0;
					break;
				case SDL_SCANCODE_D:
					rStruct->p1_right = 0;
					break;
				case SDL_SCANCODE_W:
					rStruct->p1_shoot = 0;
					break;
				default: break;
				}
			}
		}
	}
}
