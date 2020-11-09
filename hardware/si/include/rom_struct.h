#ifndef ROM_STRUCT
#define ROM_STRUCT

#include "taito_struct.h"

#include <pthread.h>
#include <stdint.h>
struct rom_struct
{
	// Keystates.  The player controls should be
	// toggled to on on press, and toggled to off on release.
	uint32_t p1_start : 1, p1_shoot : 1, p1_left : 1, p1_right : 1,
			p2_start : 1, p2_shoot : 1, p2_left : 1, p2_right : 1,
			tilt : 1, coin : 1, sound_off : 1, dip0 : 1, dip1 : 1,
			dip2 : 1, dip3 : 1, dip4 : 1, dip5 : 1, dip6 : 1,
			dip7 : 1,
			// sounds are toggled by hw_out
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
