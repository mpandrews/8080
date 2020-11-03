#ifndef ROM_STRUCT
#define ROM_STRUCT

#include "taito_struct.h"

#include <pthread.h>
#include <stdint.h>
struct rom_struct
{
	pthread_mutex_t* keystate_lock;
	pthread_mutex_t* sound_lock;
	pthread_mutex_t* reset_quit_lock;
	uint8_t* reset_flag;
	uint8_t* quit_flag;

	// Keystates.  The player controls should be
	// toggled to on on press, and toggled to off on release.
	uint32_t p1_start : 1, p1_shoot : 1, p1_left : 1, p1_right : 1,
			p2_start : 1, p2_shoot : 1, p2_left : 1, p2_right : 1,
			tilt : 1, coin : 1,
			// The DIPs should be treated as toggles:
			// each time the key is pressed anew, the stage should
			// flip.
			dip0 : 1, dip1 : 1, dip2 : 1, dip3 : 1, dip4 : 1,
			dip5 : 1, dip6 : 1, dip7 : 1,
			// ufo_sound is toggled by hw_out; all other sounds are
			// set by hw_out and then cleared by front_end after
			// reading
			ufo_sound : 1, player_die_sound : 1,
			invader_killed_sound : 1, fast_invader1_sound : 1,
			fast_invader2_sound : 1, fast_invader3_sound : 1,
			fast_invader4_sound : 1, shot_sound : 1,
			ufo_hit_sound : 1;
	uint8_t shift_old;
	uint8_t shift_new;
	uint8_t shift_offset;
};

#endif
