#include "rom_struct.h"
#include "sound.h"

#include <SDL_mixer.h>

Mix_Chunk** load_sound(void* rom_struct)
{
	struct rom_struct* rStruct = (struct rom_struct*) rom_struct;

	// Convert UFO sound to mix music
	SDL_RWops* ufo_rw    = SDL_RWFromConstMem(ufo, sizeof(ufo));
	Mix_Music* ufo_sound = Mix_LoadMUS_RW(ufo_rw, sizeof(ufo));
	if (ufo_sound == NULL)
	{ fprintf(stderr, "Could not open music: %s\n", SDL_GetError()); }

	// Init UFO sound, and pause if ufo_sound bit not set. UFO sound
	// is toggled by hw_output.
	Mix_PlayMusic(ufo_sound, -1);
	if (!rStruct->ufo_sound) { Mix_PauseMusic(); }

	// Init array of pointers to SI sound effects
	Mix_Chunk** sound_effects =
			(Mix_Chunk**) malloc(8 * sizeof(Mix_Chunk*));
	int count = 0;

	// sound_effects[0] = invader1_sound mix chunk
	SDL_RWops* invader1_rw = SDL_RWFromConstMem(invader1, sizeof(invader1));
	Mix_Chunk* invader1_sound = Mix_LoadWAV_RW(invader1_rw, 1);
	if (invader1_sound == NULL)
	{ fprintf(stderr, "Could not open invader1: %s\n", SDL_GetError()); }
	sound_effects[count++] = invader1_sound;

	// sound_effects[1] = invader2_sound mix chunk
	SDL_RWops* invader2_rw = SDL_RWFromConstMem(invader2, sizeof(invader2));
	Mix_Chunk* invader2_sound = Mix_LoadWAV_RW(invader2_rw, 1);
	if (invader2_sound == NULL)
	{ fprintf(stderr, "Could not open invader2: %s\n", SDL_GetError()); }
	sound_effects[count++] = invader2_sound;

	// sound_effects[2] = invader3_sound mix chunk
	SDL_RWops* invader3_rw = SDL_RWFromConstMem(invader3, sizeof(invader3));
	Mix_Chunk* invader3_sound = Mix_LoadWAV_RW(invader3_rw, 1);
	if (invader3_sound == NULL)
	{ fprintf(stderr, "Could not open invader3: %s\n", SDL_GetError()); }
	sound_effects[count++] = invader3_sound;

	// sound_effects[3] = invader4_sound mix chunk
	SDL_RWops* invader4_rw = SDL_RWFromConstMem(invader4, sizeof(invader4));
	Mix_Chunk* invader4_sound = Mix_LoadWAV_RW(invader4_rw, 1);
	if (invader4_sound == NULL)
	{ fprintf(stderr, "Could not open invader4: %s\n", SDL_GetError()); }
	sound_effects[count++] = invader4_sound;

	// sound_effects[4] = invader_killed_sound mix chunk
	SDL_RWops* invader_killed_rw = SDL_RWFromConstMem(
			invader_killed, sizeof(invader_killed));
	Mix_Chunk* invader_killed_sound = Mix_LoadWAV_RW(invader_killed_rw, 1);
	if (invader_killed_sound == NULL)
	{
		fprintf(stderr,
				"Could not open invader_killed: %s\n",
				SDL_GetError());
	}
	sound_effects[count++] = invader_killed_sound;

	// sound_effects[5] = player_die_sound mix chunk
	SDL_RWops* player_die_rw =
			SDL_RWFromConstMem(player_die, sizeof(player_die));
	Mix_Chunk* player_die_sound = Mix_LoadWAV_RW(player_die_rw, 1);
	if (player_die_sound == NULL)
	{
		fprintf(stderr,
				"Could not open player_die: %s\n",
				SDL_GetError());
	}
	sound_effects[count++] = player_die_sound;

	// sound_effects[6] = shot_sound mix chunk
	SDL_RWops* shot_rw    = SDL_RWFromConstMem(shot, sizeof(shot));
	Mix_Chunk* shot_sound = Mix_LoadWAV_RW(shot_rw, 1);
	if (shot_sound == NULL)
	{ fprintf(stderr, "Could not open shot: %s\n", SDL_GetError()); }
	sound_effects[count++] = shot_sound;

	// sound_effects[7] = ufo_hit_sound mix chunk
	SDL_RWops* ufo_hit_rw	 = SDL_RWFromConstMem(ufo_hit, sizeof(ufo_hit));
	Mix_Chunk* ufo_hit_sound = Mix_LoadWAV_RW(ufo_hit_rw, 1);
	if (ufo_hit_sound == NULL)
	{ fprintf(stderr, "Could not open ufo_hit: %s\n", SDL_GetError()); }
	sound_effects[count] = ufo_hit_sound;

	return sound_effects;
}

void play_sound(void* sounds, void* rom_struct)
{
	struct rom_struct* rStruct = (struct rom_struct*) rom_struct;
	Mix_Chunk** sound_effects  = (Mix_Chunk**) sounds;

	if (rStruct->ufo_sound && Mix_PausedMusic() > 0) { Mix_ResumeMusic(); }
	if (!rStruct->ufo_sound && Mix_PausedMusic() == 0) { Mix_PauseMusic(); }
	if (rStruct->fast_invader1_sound)
	{
		Mix_PlayChannel(-1, sound_effects[0], 0);
		rStruct->fast_invader1_sound = 0;
	}
	if (rStruct->fast_invader2_sound)
	{
		Mix_PlayChannel(-1, sound_effects[1], 0);
		rStruct->fast_invader2_sound = 0;
	}
	if (rStruct->fast_invader3_sound)
	{
		Mix_PlayChannel(-1, sound_effects[2], 0);
		rStruct->fast_invader3_sound = 0;
	}
	if (rStruct->fast_invader4_sound)
	{
		Mix_PlayChannel(-1, sound_effects[3], 0);
		rStruct->fast_invader4_sound = 0;
	}
	if (rStruct->invader_killed_sound)
	{
		Mix_PlayChannel(-1, sound_effects[4], 0);
		rStruct->invader_killed_sound = 0;
	}
	if (rStruct->player_die_sound)
	{
		Mix_PlayChannel(-1, sound_effects[5], 0);
		rStruct->player_die_sound = 0;
	}
	if (rStruct->shot_sound)
	{
		Mix_PlayChannel(-1, sound_effects[6], 0);
		rStruct->shot_sound = 0;
	}
	if (rStruct->ufo_hit_sound)
	{
		Mix_PlayChannel(-1, sound_effects[7], 0);
		rStruct->ufo_hit_sound = 0;
	}
}
