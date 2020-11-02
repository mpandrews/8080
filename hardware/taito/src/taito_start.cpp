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
	for (;;)
	{
		screen.videoRamToTaitoBuffer(TOP);
		screen_timer();
		screen.sendInterrupt(RST1);

		screen.videoRamToTaitoBuffer(BOTTOM);
		if (update_keystates(tStruct->rom_struct)) return 0;
		screen_timer();
		screen.sendInterrupt(RST2);
		screen.renderFrame();
	}
}
