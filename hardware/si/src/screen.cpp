#include "../include/screen.hpp"

#include <iostream>

Screen::Screen(const unsigned char* SIVideoBuffer)
{
	SDL_Init(SDL_INIT_VIDEO);
	this->SIVideoBuffer = SIVideoBuffer;
	/* this displayBuffer will contain a translation of the space invader's
	 * video RAM. The video RAM is one bit per pixel, but this buffer
	 * will contain one byte per pixel. This will allow us to impart some
	 * color information as well as on/off status. Later we may be able to
	 * alter this to either save some buffer space or add alpha channels,
	 * or other effects to more accurately simulate an analog hardware
	 * screen like that in the space invaders cabinet.
	 */
	this->displayBuffer = new Uint8[SI_SCREEN_WIDTH * SI_SCREEN_HEIGHT];

	/* Color filters. Colors are determined by the contents of each byte
	 * in the buffer. The most significant 3 bits set the red component,
	 * the next 3 significant bits set green, and the 2 least significant
	 * bits determine the blue component.
	 */
	this->topScreenColorFilter    = 0b11100000; // very red
	this->bottomScreenColorFilter = 0b00011100; // very green
	this->topScreenFilterEnd      = 38;
	this->bottomScreenFilterBegin = SI_SCREEN_WIDTH - 50;
	this->currentScreenSide	      = TOP;

	// The window is the SDL object that gets a window from the OS and
	// manages it.
	this->window = SDL_CreateWindow("Space Invaders", // window name
			SDL_WINDOWPOS_CENTERED,		  // horizontal pos
			SDL_WINDOWPOS_CENTERED,		  // vertical pos
			SI_SCREEN_WIDTH * 3,		  // width
			SI_SCREEN_HEIGHT * 3,		  // height
			(SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS) // flags
	);
	if (window == NULL)
	{
		std::cout << "Could not load SDL window. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}

	// An SDL renderer is associated with a window. It is the object that
	// refreshes the window or sections of the window
	this->renderer = SDL_CreateRenderer(
			this->window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
	{
		std::cout << "Could not load SDL renderer. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}

	// An SDL surface is needed to manage memory-mapped pixel data. When
	// a screen refresh is needed, the surface object is used in conjunction
	// with an SDL_Texture object (texture objects are transient so one is
	// not declared/maintained here) to hand off the memory-mapped data to
	// the renderer.
	this->surface = SDL_CreateRGBSurfaceWithFormatFrom(this->displayBuffer,
			SI_SCREEN_WIDTH,
			SI_SCREEN_HEIGHT,
			0,
			256,
			SDL_PIXELFORMAT_RGB332);
	if (surface == NULL)
	{
		std::cout << "Could not load SDL surface. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}
}

Screen::~Screen()
{
	delete[] this->displayBuffer;
	SDL_DestroyWindow(this->window);
	SDL_DestroyRenderer(this->renderer);
}

void Screen::translateVideoRamToBuffer()
{
	Uint8 bitMasks[8] = {0b10000000,
			0b01000000,
			0b00100000,
			0b00010000,
			0b00001000,
			0b00000100,
			0b00000010,
			0b00000001};

	Uint8 colorPixelValue;
	Uint8 pixels;

	/* This will translate the 1 bit per pixel SI video ram buffer to the
	 * 1 byte per pixel video buffer for our display. This will not perform
	 * the rotation. That will be handled by SDL when it comes time to
	 * render the screen. Probably much more efficiently than I could do
	 * here.
	 *
	 * This will also impart color information into the bytes. Since this
	 * is happening pre-rotation, the column we are in will determine the
	 * color representation. The rotation is counter-clockwise, so the
	 * left-most columns will become the bottom of the screen and the right-
	 * most columns will become the top.
	 */

	// iterate through each byte of the space invader's video buffer
	for (int i = 0; i < (SI_SCREEN_WIDTH * SI_SCREEN_HEIGHT) / 8; ++i)
	{
		pixels = this->SIVideoBuffer[i];
		// expand each byte into one byte for each of the 8 bits
		for (int j = 0; j < 8; ++j)
		{
			// determine and set color for this byte
			if (pixels & bitMasks[j])
			{
				int col = ((i + 1) * 8) / SI_SCREEN_HEIGHT;
				if (col < this->topScreenFilterEnd)
					colorPixelValue =
							this->topScreenColorFilter;
				else if (col > this->bottomScreenFilterBegin)
					colorPixelValue =
							this->bottomScreenColorFilter;
				else
					colorPixelValue = WHITE_PIXEL;
			}
			else
				colorPixelValue = BLACK_PIXEL;
			this->displayBuffer[i * 8 + j] = colorPixelValue;
		}
	}
}

void Screen::renderFrame()
{
	// generate a texture from the surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(
			this->renderer, this->surface);
	if (texture == NULL)
	{
		std::cout << "Could not create texture. " << SDL_GetError()
			  << std::endl;
	}

	// clear the previous render and display the new one
	SDL_RenderClear(this->renderer);
	if (SDL_RenderCopy(this->renderer, texture, NULL, NULL) < 0)
	{
		std::cout << "Error rendering texture. " << SDL_GetError()
			  << std::endl;
	}
	SDL_RenderPresent(this->renderer);
	SDL_DestroyTexture(texture);
}
