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
		if (update_keystates(tStruct->rom_struct)) return 0;
		play_sound(sound_effects, tStruct->rom_struct);
		screen_timer();
		screen.sendInterrupt(RST2);
		screen.renderFrame();
	}
}
