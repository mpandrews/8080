extern "C"
{
#include "taito_start.h"

#include "screen_timer.h"
}
#include "hw_lib_imports.h"
#include "taitoScreen.hpp"

extern "C" int
taito_start(struct taito_struct* tStruct, uint8_t proms[][896], int num_proms)
{
	TaitoScreen screen(tStruct, proms, num_proms);
	sideOfScreen screenHalf;
	for (;;)
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
