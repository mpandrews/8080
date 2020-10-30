#ifndef ROM_STRUCT
#define ROM_STRUCT

#include "taito_struct.h"

#include <pthread.h>
#include <stdint.h>
struct rom_struct
{
	pthread_mutex_t* keystate_lock;
	pthread_mutex_t* reset_quit_lock;
	uint8_t* reset_flag;
	uint8_t* quit_flag;

	// Keystates.  The player controls should be
	// toggled to on on press, and toggled to off on release.
	uint32_t p1_start : 1, p1_shoot : 1, p1_left : 1, p1_right : 1,
			p2_start : 1, p2_shoot : 1, p2_left : 1, p2_right : 1,
			// Tilt and the DIPs should be treated as toggles:
			// each time the key is pressed anew, the stage should
			// flip.
			tilt : 1, dip0 : 1, dip1 : 1, dip2 : 1, dip3 : 1,
			dip4 : 1, dip5 : 1, dip6 : 1, dip7 : 1,
			// Coin and Reset should be treated as sticky when
			// reading keyboard state: they should be set when the
			// key is pressed, but not unset when it is released.
			// Instead, the CPU thread should clear them after
			// reading them.
			coin : 1,
			// Sound bits
			player_die_sound : 1, invader_killed_sound : 1,
			fast_invader1_sound : 1, fast_invader2_sound : 1,
			fast_invader3_sound : 1, fast_invader4_sound : 1,
			shot_sound : 1, ufo_sound : 1, ufo_hit_sound : 1;
	uint8_t shift_old;
	uint8_t shift_new;
	uint8_t shift_offset;
};

#endif
