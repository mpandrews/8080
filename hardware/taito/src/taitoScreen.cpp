#include "taitoScreen.hpp"

#include "hw_lib_imports.h"

#include <iostream>

TaitoScreen::TaitoScreen(struct taito_struct* tStruct,
		Uint8 colorMaskProms[][896],
		int numColorMaskProms)
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
			new Uint16[TAITO_SCREEN_WIDTH * TAITO_SCREEN_HEIGHT];

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

	/* An SDL surface is needed to manage memory-mapped pixel data. When
	 * a screen refresh is needed, the surface object is used in conjunction
	 * with an SDL_Texture object (texture objects are transient so one is
	 * not declared/maintained here) to hand off the memory-mapped data to
	 * the renderer.
	 */
	this->surface = SDL_CreateRGBSurfaceWithFormatFrom(this->displayBuffer,
			TAITO_SCREEN_WIDTH,  // width of surface
			TAITO_SCREEN_HEIGHT, // height of surface
			SDL_BYTESPERPIXEL(
					SDL_PIXELFORMAT_ARGB4444), // depth -
								   // bits-per-pixel
			TAITO_SCREEN_WIDTH * 2, // pitch - width of one row of
						// pixels
			SDL_PIXELFORMAT_RGBA4444); // format
	if (surface == NULL)
	{
		std::cout << "Could not load SDL surface. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}

	this->numColorMasks = numColorMaskProms;
	if (this->numColorMasks > 0)
	{
		this->colorMasks    = new SDL_Surface*[this->numColorMasks];
		this->currColorMask = 0;
		for (int i = 0; i < this->numColorMasks; ++i)
		{
			// translate the color mask into a pixel byte
			// array
			Uint8* maskBytes =
					getColorMaskFromProm(colorMaskProms[i]);

			this->colorMasks[i] =
					SDL_CreateRGBSurfaceWithFormatFrom(
							maskBytes,
							TAITO_SCREEN_WIDTH / 8,
							TAITO_SCREEN_HEIGHT / 8,
							8,
							TAITO_SCREEN_WIDTH / 8,
							SDL_PIXELFORMAT_RGB332);
		}
	}
	else
	{
		this->colorMasks    = NULL;
		this->currColorMask = -1;
	}
}

Uint8* TaitoScreen::getColorMaskFromProm(Uint8* prom)
{
	Uint8* mask = new Uint8[(TAITO_SCREEN_WIDTH / 8)
				* (TAITO_SCREEN_HEIGHT / 8)];
	for (int i = 0; i
			< (TAITO_SCREEN_WIDTH / 8) * (TAITO_SCREEN_HEIGHT / 8);
			++i)
	{
		switch (prom[i])
		{
		case 0:
			mask[i] = 0b00000000; // black
			break;
		case 1:
			mask[i] = 0b11100000; // red
			break;
		case 2:
			mask[i] = 0b00000011; // blue
			break;
		case 3:
			mask[i] = 0b11100011; // red + blue = magenta
			break;
		case 4:
			mask[i] = 0b00011100; // green
			break;
		case 5:
			mask[i] = 0b11111100; // red + green = yellow
			break;
		case 6:
			mask[i] = 0b00011111; // green + blue = cyan
			break;
		case 7:
			mask[i] = 0b11111111; // green + blue + red = white
			break;
		};
	}
	return mask;
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

	SDL_Texture* texture = NULL;

	texture = SDL_CreateTextureFromSurface(
			this->renderer, this->colorMasks[0]);
	if (texture == NULL)
	{
		std::cout << "Could not create texture. " << SDL_GetError()
			  << std::endl;
	}
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
	SDL_DestroyTexture(texture);

	// generate a texture from the surface
	texture = SDL_CreateTextureFromSurface(this->renderer, this->surface);
	if (texture == NULL)
	{
		std::cout << "Could not create texture. " << SDL_GetError()
			  << std::endl;
	}

	if (SDL_RenderCopyEx(this->renderer,
			    texture,
			    NULL,
			    &dest,
			    270,
			    NULL,
			    SDL_FLIP_NONE)
			< 0)
	{
		std::cout << "Error rendering color mask texture. "
			  << SDL_GetError() << std::endl;
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
		end_row	  = SCREEN_DIVIDE_ROW;
	}
	else
	{
		start_row = SCREEN_DIVIDE_ROW;
		end_row	  = TAITO_SCREEN_HEIGHT;
	}

	/* This will translate the 1 bit per pixel TAITO video ram buffer to the
	 * 1 byte per pixel video buffer for our display. This will not perform
	 * the rotation. That will be handled by SDL when it comes time to
	 * render the screen.
	 */
	for(int i = start_row; i < end_row; ++i)
	{
		for(int j = 0; j < TAITO_SCREEN_WIDTH / 8; ++j)
		{
			pixels = this->taitoVideoBuffer[i * (TAITO_SCREEN_WIDTH/8) + j];
			for(int k = 0; k < 8; ++k)
			{
				this->displayBuffer[i * TAITO_SCREEN_WIDTH + (j * 8) + k] =
					pixels & bitMasks[k] ? WHITE_PIXEL : BLACK_PIXEL;
			}
		}
	}

	auto apply_blur = [](int i, int j, Uint16* displayBuffer, Uint16 currVal)
	{
		switch (
				((i == 0) << 3) // top
				| ((i == TAITO_SCREEN_HEIGHT - 1) << 2) // bottom
				| ((j == 0) << 1) // left edge
				| (j == TAITO_SCREEN_WIDTH - 1) // right edge
		       )
		{
			case 0b1010: // top-left corner
				break;
			case 0b1000: // top edge
				break;
			case 0b1001: // top-right corner
				break;
			case 0b0001: // right edge
				break;
			case 0b0101: // bottom-right corner
				break;
			case 0b0100: // bottom edge
				break;
			case 0b0110: // bottom-left corner
				break;
			case 0b0010: // left edge
				break;
			default: // anywhere in the middle
				if(
					(displayBuffer[i * TAITO_SCREEN_WIDTH + j + 1] < currVal)
					|| (displayBuffer[i * TAITO_SCREEN_WIDTH + j -1] +1 < currVal)
					|| (displayBuffer[(i + 1) * TAITO_SCREEN_WIDTH + j] < currVal)
					|| (displayBuffer[(i - 1) * TAITO_SCREEN_WIDTH + j]+1 < currVal))
				{
					return (Uint16)(currVal - 1);
				}
				else
					return currVal;
		};
		return currVal;
	};


	// apply a slight blur
	for(int k = 0; k < 1; ++k)
	{
		for(int i = start_row; i < end_row; ++i)
		{
			for(int j = 0; j < TAITO_SCREEN_WIDTH; ++j)
			{
				Uint16* pixel = &this->displayBuffer[i * TAITO_SCREEN_WIDTH + j];
				*pixel = (*pixel == WHITE_PIXEL) ? WHITE_PIXEL : apply_blur(i, j, this->displayBuffer, *pixel) ;

			}
		}
	}
}

TaitoScreen::~TaitoScreen()
{
	delete[] this->displayBuffer;
	SDL_DestroyWindow(this->window);
	SDL_DestroyRenderer(this->renderer);
	for (int i = 0; i < this->numColorMasks; ++i)
		SDL_FreeSurface(this->colorMasks[i]);
	SDL_FreeSurface(this->surface);
	delete[] this->colorMasks;
}
