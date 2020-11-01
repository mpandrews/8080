#ifndef _TAITO_SCREEN_HPP_
#define _TAITO_SCREEN_HPP_

extern "C"
{
#include "cpu.h"
#include "screen_timer.h"
#include "taito_struct.h"
}

#include "hw_lib_imports.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#define RST1 (0xcf)
#define RST2 (0xd7)

#define TAITO_SCREEN_WIDTH  256
#define TAITO_SCREEN_HEIGHT 224
#define WINDOW_SCALE_FACTOR 3
#define WHITE_PIXEL	    0x0000
#define BLACK_PIXEL	    0x000f

// This sets line that defines the end of the top half of the screen and the
// beginning of the second half of the screen, as it pertains to interrupts.
#define SCREEN_DIVIDE_ROW 97

enum sideOfScreen
{
	TOP,
	BOTTOM
};

class TaitoScreen
{
      private:
	pthread_mutex_t* interruptLock;
	pthread_cond_t* interruptCond;
	Uint8* interruptBuffer;

	// A pointer to the beginning of the video buffer that the
	// 8080 thread will write taito video data to
	const Uint8* taitoVideoBuffer;
	// The translated version of the taitoVideoBuffer. Expanded to 1 byte
	// per pixel
	Uint16* displayBuffer;

	pthread_mutex_t* vidBufferLock;
	pthread_cond_t* vidBufferCond;

	// SDL objects used to create a window and render graphics to it
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;

	int currColorMask;
	int numColorMasks;
	SDL_Surface** colorMasks;

      public:
	TaitoScreen(struct taito_struct* tStruct,
			Uint8 colorMaskProms[][896],
			int numColorMaskProms);
	~TaitoScreen();
	void videoRamToTaitoBuffer(sideOfScreen);
	void renderFrame();
	void sendInterrupt(Uint8 interruptCode);
	Uint8* getColorMaskFromProm(Uint8*);
};

#endif
