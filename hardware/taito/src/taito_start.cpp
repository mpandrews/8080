#include "taito_start.hpp"

#include "hw_lib_imports.h"
#include "screen_timer.h"
#include "taitoScreen.hpp"

extern "C" int taito_start(struct taito_struct* tStruct)
{
	TaitoScreen screen(tStruct);
	sideOfScreen screenHalf;
	for (int i = 0;; ++i)
	{
		screenHalf = TOP;
		screen.videoRamToTaitoBuffer(screenHalf);
		screen_timer();
		screen.sendInterrupt(RST1);

		screenHalf = BOTTOM;
		screen.videoRamToTaitoBuffer(screenHalf);
		if (update_keystates(tStruct->rom_struct)) return 0;
		screen_timer();
		screen.sendInterrupt(RST2);
		screen.renderFrame();
	}
}
