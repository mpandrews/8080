#ifndef _TAITO_SCREEN_H_
#define _TAITO_SCREEN_H_

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
#define WINDOW_SCALE_FACTOR 4
#define WHITE_PIXEL	    0b11111111
#define BLACK_PIXEL	    0b00000000

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
	Uint8* displayBuffer;

	pthread_mutex_t* vidBufferLock;
	pthread_cond_t* vidBufferCond;

	// color filters for the top and bottom of the taito screen
	Uint8 topFilterColor;
	Uint8 bottomFilterColor;

	// SDL objects used to create a window and render graphics to it
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;

      public:
	TaitoScreen(struct taito_struct* tStruct);
	~TaitoScreen();
	void videoRamToTaitoBuffer(sideOfScreen);
	void renderFrame();
	void sendInterrupt(Uint8 interruptCode);
};

#endif
