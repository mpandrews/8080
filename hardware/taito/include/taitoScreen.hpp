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
#include <SDL2/SDL_mixer.h>

#define RST1 (0xcf)
#define RST2 (0xd7)

#define TAITO_SCREEN_WIDTH  256
#define TAITO_SCREEN_HEIGHT 224
#define WINDOW_SCALE_FACTOR 3
#define TRANSPARENT_PIXEL   0x0003
#define BLACK_PIXEL	    0x000f

// some helper function defines for the class
#define GET_PIXEL_VALUE_RIGHT(pix_ptr) (*(pix_ptr + 1))
#define GET_PIXEL_VALUE_BELOW(pix_ptr) (*(pix_ptr + TAITO_SCREEN_WIDTH))
#define GET_PIXEL_VALUE_LEFT(pix_ptr)  (*(pix_ptr - 1))
#define GET_PIXEL_VALUE_ABOVE(pix_ptr) (*(pix_ptr - TAITO_SCREEN_WIDTH))
#define GET_PIXEL_VALUE_ABOVE_RIGHT(pix_ptr) \
	(*(pix_ptr - TAITO_SCREEN_WIDTH + 1))
#define GET_PIXEL_VALUE_ABOVE_LEFT(pix_ptr) \
	(*(pix_ptr - TAITO_SCREEN_WIDTH - 1))
#define GET_PIXEL_VALUE_BELOW_RIGHT(pix_ptr) \
	(*(pix_ptr + TAITO_SCREEN_WIDTH + 1))
#define GET_PIXEL_VALUE_BELOW_LEFT(pix_ptr) \
	(*(pix_ptr + TAITO_SCREEN_WIDTH - 1))

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
	struct taito_struct* const tStruct;

	pthread_mutex_t* const interruptLock;
	pthread_cond_t* const interruptCond;
	Uint8* const interruptBuffer;

	// A pointer to the beginning of the video buffer that the
	// 8080 thread will write taito video data to
	const Uint8* const taitoVideoBuffer;
	// The translated version of the taitoVideoBuffer. Expanded to 1 byte
	// per pixel
	Uint16* displayBuffer;

	pthread_mutex_t* const vidBufferLock;
	pthread_cond_t* const vidBufferCond;
	pthread_mutex_t* const keystateLock;
	pthread_mutex_t* const resetQuitLock;

	// SDL objects used to create a window and render graphics to it
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* gameDisplaySurface;

	int currColorMask;
	const int numColorMasks;
	SDL_Surface** colorMasks;

      public:
	// constructor destructor
	TaitoScreen(struct taito_struct* tStruct);
	~TaitoScreen();

	// getters setters
	void setCurrColorMask(int);
	int getCurrColorMask();

	// managing frame renders
	void videoRamToTaitoBuffer(sideOfScreen);
	void renderFrame();
	void renderSurface(SDL_Surface*);
	void applyBlur();
	int handleInput();

	// other
	void sendInterrupt(Uint8 interruptCode);
	Uint8* getColorMaskFromProm(const unsigned char* const);
};

#endif
