#include "taitoScreen.hpp"

#include "hw_lib_imports.h"

#include <iostream>

TaitoScreen::TaitoScreen(struct taito_struct* tStruct)
{
	// set members from taito_struct
	this->interruptLock    = tStruct->interrupt_lock;
	this->interruptCond    = tStruct->interrupt_cond;
	this->interruptBuffer  = tStruct->interrupt_buffer;
	this->vidBufferCond    = tStruct->vbuffer_cond;
	this->vidBufferLock    = tStruct->vbuffer_lock;
	this->taitoVideoBuffer = tStruct->vbuffer;
	pthread_mutex_lock(this->vidBufferLock);

	/* this displayBuffer will contain a translation of the space invader's
	 * video RAM. The video RAM is one bit per pixel, but this buffer
	 * will contain one byte per pixel. This will allow us to impart some
	 * color information as well as on/off status. Later we may be able to
	 * alter this to either save some buffer space or add alpha channels,
	 * or other effects to more accurately simulate an analog hardware
	 * screen like that in the space invaders cabinet.
	 */
	this->displayBuffer =
			new Uint8[TAITO_SCREEN_WIDTH * TAITO_SCREEN_HEIGHT];

	/* Color filters. Colors are determined by the contents of each byte
	 * in the buffer. The most significant 3 bits set the red component,
	 * the next 3 significant bits set green, and the 2 least significant
	 * bits determine the blue component.
	 */
	this->currentScreenSide = TOP;

	// Now we set up and configure SDL to render to a window
	SDL_Init(SDL_INIT_VIDEO);
	this->window = SDL_CreateWindow("Space Invaders", // window name
			SDL_WINDOWPOS_CENTERED,		  // horizontal pos
			SDL_WINDOWPOS_CENTERED,		  // vertical pos
			TAITO_SCREEN_HEIGHT * WINDOW_SCALE_FACTOR,  // width
			TAITO_SCREEN_WIDTH * WINDOW_SCALE_FACTOR,   // height
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
			TAITO_SCREEN_WIDTH,	 // width of surface
			TAITO_SCREEN_HEIGHT,	 // height of surface
			8,			 // depth - bits-per-pixel
			TAITO_SCREEN_WIDTH,	 // pitch - width of one row of
						 // pixels
			SDL_PIXELFORMAT_RGB332); // format
	if (surface == NULL)
	{
		std::cout << "Could not load SDL surface. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}
}

void TaitoScreen::sendInterrupt(Uint8 interruptCode)
{
	pthread_mutex_lock(this->interruptLock);
	if (*this->interruptBuffer)
		pthread_cond_wait(this->interruptCond, this->interruptLock);
	*this->interruptBuffer = interruptCode;
	pthread_mutex_unlock(this->interruptLock);
	pthread_cond_wait(this->vidBufferCond, this->vidBufferLock);
}

void TaitoScreen::renderFrame()
{
	// generate a texture from the surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(
			this->renderer, this->surface);
	if (texture == NULL)
	{
		std::cout << "Could not create texture. " << SDL_GetError()
			  << std::endl;
	}

	// Create SDL_Rect to scale the rotated screen to our window
	SDL_Rect dest;
	dest.x = (WINDOW_SCALE_FACTOR
				 * (TAITO_SCREEN_HEIGHT - TAITO_SCREEN_WIDTH))
		 / 2;
	dest.y = (WINDOW_SCALE_FACTOR
				 * (TAITO_SCREEN_WIDTH - TAITO_SCREEN_HEIGHT))
		 / 2;
	dest.w = WINDOW_SCALE_FACTOR * TAITO_SCREEN_WIDTH;
	dest.h = WINDOW_SCALE_FACTOR * TAITO_SCREEN_HEIGHT;

	// clear the previous render and display the new one
	SDL_RenderClear(this->renderer);
	if (SDL_RenderCopyEx(this->renderer,
			    texture,
			    NULL,
			    &dest,
			    270,
			    NULL,
			    SDL_FLIP_NONE)
			< 0)
	{
		std::cout << "Error rendering texture. " << SDL_GetError()
			  << std::endl;
	}
	SDL_RenderPresent(this->renderer);
	SDL_DestroyTexture(texture);
}

void TaitoScreen::videoRamToTaitoBuffer(sideOfScreen screenHalf)
{
	Uint8 bitMasks[8] = {0b00000001,
			0b00000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b10000000};
	Uint8 pixels;
	int start_row, end_row;

	if (screenHalf == TOP)
	{
		start_row = 0;
		end_row	  = SCREEN_SPLIT_ROW;
	}
	else
	{
		start_row = SCREEN_SPLIT_ROW;
		end_row	  = TAITO_SCREEN_HEIGHT;
	}

	/* This will translate the 1 bit per pixel TAITO video ram buffer to the
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
	for (int i = TAITO_SCREEN_WIDTH * start_row / 8;
			i < TAITO_SCREEN_WIDTH * end_row / 8;
			++i)
	{
		pixels = this->taitoVideoBuffer[i];
		// expand each byte into one byte for each of the 8 bits
		for (int j = 0; j < 8; ++j)
		{
			this->displayBuffer[(i * 8) + j] =
					pixels & bitMasks[j] ? WHITE_PIXEL
							     : BLACK_PIXEL;
		}
	}
}

TaitoScreen::~TaitoScreen()
{
	delete[] this->displayBuffer;
	SDL_DestroyWindow(this->window);
	SDL_DestroyRenderer(this->renderer);
}
