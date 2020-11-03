#ifndef PLAY_SOUND_H
#define PLAY_SOUND_H

#include <SDL_mixer.h>

Mix_Chunk** load_sound(void* rom_struct);
void play_sound(void* sounds, void* rom_struct);

#endif
