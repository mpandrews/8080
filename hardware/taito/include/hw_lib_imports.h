#ifndef HW_LIB_IMPORTS_H
#define HW_LIB_IMPORTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

__attribute__((weak)) extern void update_keystates(void*, SDL_Event*);
__attribute__((weak)) extern Mix_Chunk** load_sound();
__attribute__((weak)) extern void play_sound(void* sounds, void* rom_struct);

#endif
