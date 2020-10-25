#ifndef _SCREEN_DISPLAY_H_
#define _SCREEN_DISPLAY_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#define SI_SCREEN_WIDTH 256 
#define SI_SCREEN_HEIGHT 224 
#define BOTTOM_FILTER_WIDTH 64
#define TOP_FILTER_WIDTH 40
#define WINDOW_SCALE_FACTOR 4 
#define WHITE_PIXEL 0b11111111
#define BLACK_PIXEL 0b00000000

// This sets line that defines the end of the top half of the screen and the
// beginning of the second half of the screen, as it pertains to interrupts.
#define SCREEN_DIVIDE_ROW 97

enum sideOfScreen {TOP, BOTTOM};

class Screen
{
	private:
		// A pointer to the beginning of the video buffer that the
		// 8080 thread will write SI video data to
		const Uint8* SIVideoBuffer;
		/* This display buffer is privately managed. It contains an
		 * interpreted version of the Space invaders RAM dedicated to
		 * graphics. This buffer has one byte per pixel and includes
		 * color information as well.
		 */
		Uint8 *displayBuffer;
		sideOfScreen currentScreenSide;

		// color filters for the top and bottom of the SI screen
		Uint8 topFilterColor;
		Uint8 bottomFilterColor;
		int topFilterWidth;
		int bottomFilterWidth;

		// SDL objects used to create a window and render graphics to it
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Surface* surface;


	public:
		Screen(const unsigned char*);
		~Screen();
		void translateVideoRamToBuffer();
		void renderFrame();
};

#endif
