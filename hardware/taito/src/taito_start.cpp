extern "C"
{
#include "taito_start.h"
#include "screen_timer.h"
}

#include "hw_lib_imports.h"
#include "taitoScreen.hpp"

extern "C" int taito_start(struct taito_struct* tStruct)
{
	TaitoScreen screen(tStruct);
	Mix_Chunk** sound_effects = load_sound();

	for (;;)
	{
		screen.videoRamToTaitoBuffer(TOP);
		screen_timer();
		screen.sendInterrupt(RST1);

		screen.videoRamToTaitoBuffer(BOTTOM);
		if (update_keystates(tStruct)) return 0;

		pthread_mutex_lock(tStruct->sound_lock);
		play_sound(sound_effects, tStruct->rom_struct);
		pthread_mutex_unlock(tStruct->sound_lock);

		screen_timer();
		screen.sendInterrupt(RST2);
		screen.renderFrame();
	}
}
