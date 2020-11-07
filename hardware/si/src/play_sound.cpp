#include "rom_struct.h"
#include "sound.h"

#include <SDL2/SDL_mixer.h>

Mix_Chunk** load_sound()
{
	// Init UFO sound. UFO sound is toggled by hw_output.
	SDL_RWops* ufo_rw    = SDL_RWFromConstMem(ufo, sizeof(ufo));
	Mix_Music* ufo_sound = Mix_LoadMUS_RW(ufo_rw, sizeof(ufo));
	Mix_PlayMusic(ufo_sound, -1);

	// Init array of pointers to SI sound effects
	Mix_Chunk** sound_effects =
			(Mix_Chunk**) malloc(8 * sizeof(Mix_Chunk*));

	SDL_RWops* invader1_rw = SDL_RWFromConstMem(invader1, sizeof(invader1));
	Mix_Chunk* invader1_sound = Mix_LoadWAV_RW(invader1_rw, 1);
	sound_effects[0]	  = invader1_sound;

	SDL_RWops* invader2_rw = SDL_RWFromConstMem(invader2, sizeof(invader2));
	Mix_Chunk* invader2_sound = Mix_LoadWAV_RW(invader2_rw, 1);
	sound_effects[1]	  = invader2_sound;

	SDL_RWops* invader3_rw = SDL_RWFromConstMem(invader3, sizeof(invader3));
	Mix_Chunk* invader3_sound = Mix_LoadWAV_RW(invader3_rw, 1);
	sound_effects[2]	  = invader3_sound;

	SDL_RWops* invader4_rw = SDL_RWFromConstMem(invader4, sizeof(invader4));
	Mix_Chunk* invader4_sound = Mix_LoadWAV_RW(invader4_rw, 1);
	sound_effects[3]	  = invader4_sound;

	SDL_RWops* invader_killed_rw = SDL_RWFromConstMem(
			invader_killed, sizeof(invader_killed));
	Mix_Chunk* invader_killed_sound = Mix_LoadWAV_RW(invader_killed_rw, 1);
	sound_effects[4]		= invader_killed_sound;

	SDL_RWops* player_die_rw =
			SDL_RWFromConstMem(player_die, sizeof(player_die));
	Mix_Chunk* player_die_sound = Mix_LoadWAV_RW(player_die_rw, 1);
	sound_effects[5]	    = player_die_sound;

	SDL_RWops* shot_rw    = SDL_RWFromConstMem(shot, sizeof(shot));
	Mix_Chunk* shot_sound = Mix_LoadWAV_RW(shot_rw, 1);
	sound_effects[6]      = shot_sound;

	SDL_RWops* ufo_hit_rw	 = SDL_RWFromConstMem(ufo_hit, sizeof(ufo_hit));
	Mix_Chunk* ufo_hit_sound = Mix_LoadWAV_RW(ufo_hit_rw, 1);
	sound_effects[7]	 = ufo_hit_sound;

	if (!invader1_sound || !invader2_sound || !invader3_sound
			|| !invader4_sound || !invader_killed_sound
			|| !player_die_sound || !shot_sound || !ufo_hit_sound
			|| !ufo_sound)
	{ fprintf(stderr, "Sound effect loading error\n"); }

	return sound_effects;
}

void play_sound(void* sounds, void* rom_struct)
{
	struct rom_struct* rStruct = (struct rom_struct*) rom_struct;
	Mix_Chunk** sound_effects  = (Mix_Chunk**) sounds;

	if (rStruct->sound_off) { return; }

	if (rStruct->ufo_sound) { Mix_ResumeMusic(); }
	else
	{
		Mix_PauseMusic();
	}

	if (rStruct->fast_invader1_sound && !Mix_Playing(0))
	{ Mix_PlayChannel(0, sound_effects[0], 0); }
	if (rStruct->fast_invader2_sound && !Mix_Playing(1))
	{ Mix_PlayChannel(1, sound_effects[1], 0); }
	if (rStruct->fast_invader3_sound && !Mix_Playing(2))
	{ Mix_PlayChannel(2, sound_effects[2], 0); }
	if (rStruct->fast_invader4_sound && !Mix_Playing(3))
	{ Mix_PlayChannel(3, sound_effects[3], 0); }
	if (rStruct->invader_killed_sound && !Mix_Playing(4))
	{ Mix_PlayChannel(4, sound_effects[4], 0); }
	if (rStruct->player_die_sound && !Mix_Playing(5))
	{ Mix_PlayChannel(5, sound_effects[5], 0); }
	if (rStruct->shot_sound && !Mix_Playing(6))
	{ Mix_PlayChannel(6, sound_effects[6], 0); }
	if (rStruct->ufo_hit_sound && !Mix_Playing(7))
	{ Mix_PlayChannel(7, sound_effects[7], 0); }
}
