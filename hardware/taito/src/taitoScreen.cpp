#include "taitoScreen.hpp"

#include "hw_lib_imports.h"

#include <iostream>

TaitoScreen::TaitoScreen(struct taito_struct* tStruct)
    : tStruct(tStruct), interruptLock(tStruct->interrupt_lock),
      interruptCond(tStruct->interrupt_cond),
      interruptBuffer(tStruct->interrupt_buffer),
      taitoVideoBuffer(tStruct->vbuffer), vidBufferLock(tStruct->vbuffer_lock),
      vidBufferCond(tStruct->vbuffer_cond),
      keystateLock(tStruct->keystate_lock),
      resetQuitLock(tStruct->reset_quit_lock), numColorMasks(tStruct->num_proms)
{
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
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		std::cout << "Could not initialize SDL. " << std::endl;
		exit(1);
	};
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

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 512) != 0)
	{
		fprintf(stderr,
				"Unable to initialize audio: %s\n",
				Mix_GetError());
		exit(1);
	}
	Mix_AllocateChannels(9);
	Mix_ReserveChannels(9);
	Mix_Volume(6, 64);

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
	this->gameDisplaySurface = SDL_CreateRGBSurfaceWithFormatFrom(
			this->displayBuffer,
			TAITO_SCREEN_WIDTH,  // width of surface
			TAITO_SCREEN_HEIGHT, // height of surface
			SDL_BYTESPERPIXEL(
					SDL_PIXELFORMAT_ARGB4444), // depth -
								   // bits-per-pixel
			TAITO_SCREEN_WIDTH * 2, // pitch - width of one row of
						// pixels
			SDL_PIXELFORMAT_RGBA4444); // format
	if (gameDisplaySurface == NULL)
	{
		std::cout << "Could not load SDL surface. " << SDL_GetError()
			  << std::endl;
		exit(1);
	}

	if (this->numColorMasks > 0)
	{
		this->colorMasks    = new SDL_Surface*[this->numColorMasks];
		this->currColorMask = 0;
		for (int i = 0; i < this->numColorMasks; ++i)
		{
			// translate the color mask into a pixel byte
			// array
			Uint8* maskBytes =
					getColorMaskFromProm(tStruct->proms[i]);

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

Uint8* TaitoScreen::getColorMaskFromProm(const unsigned char* const prom)
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

int TaitoScreen::getCurrColorMask() { return this->currColorMask; }

void TaitoScreen::setCurrColorMask(int maskIndex)
{
	if (this->colorMasks != NULL)
		if (maskIndex >= 0 && maskIndex < this->numColorMasks)
			this->currColorMask = maskIndex;
	// default here: silently ignore bad input for now. Maybe it should
	// return a status success/fail value?
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

void TaitoScreen::applyBlur()
{
	Uint16 newBuff[TAITO_SCREEN_HEIGHT * TAITO_SCREEN_WIDTH];

	/* this lambda function determines the value of the alpha channel for a
	 * single pixel based upon the transparency of its surrounding pixels.
	 *
	 * Some background that is needed to understand how it works:
	 *
	 * About our pixel format: We're using a 16-bit pixel format where each
	 * pixel is encoded like so: 0b(RRRRGGGGBBBBAAAA) so the least
	 * significant four bits comprises the alpha channel value and the most
	 * significant foru bits comprises the red channel value, for example.
	 *
	 * Meanwhile, we only have two possible pixel values in our buffer:
	 * black (0x000f, i.e. no red, green, or blue, and opaque) and
	 * transparent (0x0000, i.e. no r, g, b, and no alpha).
	 *
	 * So, this is how the 'algorithm' goes:
	 *    1. Calculate the total alpha values of all surrounding pixels. We
	 *    want to end up with a total of adjacent ON (transparent) pixels
	 *    because we'll use the number of adjacent on pixels to set the
	 *    transparency of the given pixel
	 *    2. Return a pixel value that is mapped to the number of adjacent
	 *    transparent pixels
	 */
	auto getPixelBlurValue = [](int row, int col, Uint16* currVal) {
		int adjacentBlackPixels = 0;

		// switch on an amalgamated conditional value that determines
		// whether the pixel is on an edge and if so, which edge
		switch (((row == 0) << 3) // top
				| ((row == TAITO_SCREEN_HEIGHT - 1)
						<< 2)		  // bottom
				| ((col == 0) << 1)		  // left edge
				| (col == TAITO_SCREEN_WIDTH - 1) // right edge
		)
		{
			// get the total of surrounding pixels
		case 0b1010: // top-left corner
			adjacentBlackPixels =
					5
					+ (GET_PIXEL_VALUE_RIGHT(currVal)
							  + GET_PIXEL_VALUE_BELOW_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b1000: // top edge
			adjacentBlackPixels =
					3
					+ (GET_PIXEL_VALUE_RIGHT(currVal)
							  + GET_PIXEL_VALUE_BELOW_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_LEFT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b1001: // top-right corner
			adjacentBlackPixels =
					5
					+ (GET_PIXEL_VALUE_BELOW(currVal)
							  + GET_PIXEL_VALUE_BELOW_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_LEFT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b0001: // right edge
			adjacentBlackPixels =
					3
					+ (GET_PIXEL_VALUE_ABOVE(currVal)
							  + GET_PIXEL_VALUE_BELOW(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_ABOVE_LEFT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b0101: // bottom-right corner
			adjacentBlackPixels =
					5
					+ (GET_PIXEL_VALUE_ABOVE(currVal)
							  + GET_PIXEL_VALUE_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_ABOVE_LEFT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b0100: // bottom edge
			adjacentBlackPixels =
					3
					+ (GET_PIXEL_VALUE_ABOVE(currVal)
							  + GET_PIXEL_VALUE_ABOVE_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_LEFT(
									  currVal)
							  + GET_PIXEL_VALUE_ABOVE_LEFT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b0110: // bottom-left corner
			adjacentBlackPixels =
					5
					+ (GET_PIXEL_VALUE_ABOVE(currVal)
							  + GET_PIXEL_VALUE_ABOVE_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_RIGHT(
									  currVal))
							  / BLACK_PIXEL;
			break;
		case 0b0010: // left edge
			adjacentBlackPixels =
					3
					+ (GET_PIXEL_VALUE_ABOVE(currVal)
							  + GET_PIXEL_VALUE_ABOVE_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW_RIGHT(
									  currVal)
							  + GET_PIXEL_VALUE_BELOW(
									  currVal))
							  / 15;
			break;
		default: // anywhere in the middle
			adjacentBlackPixels =
					(GET_PIXEL_VALUE_ABOVE(currVal)
							+ GET_PIXEL_VALUE_ABOVE_RIGHT(
									currVal)
							+ GET_PIXEL_VALUE_RIGHT(
									currVal)
							+ GET_PIXEL_VALUE_BELOW_RIGHT(
									currVal)
							+ GET_PIXEL_VALUE_BELOW(
									currVal)
							+ GET_PIXEL_VALUE_BELOW_LEFT(
									currVal)
							+ GET_PIXEL_VALUE_LEFT(
									currVal)
							+ GET_PIXEL_VALUE_ABOVE_LEFT(
									currVal))
					/ BLACK_PIXEL;
		};

		// we'll switch on the inverse of the number of adjacent black
		// pixels. so case 1 == one adjacent transparent pixel
		switch (8 - adjacentBlackPixels)
		{
		case 1: return 0x0d;
		case 2: return 0x0d;
		case 3: return 0x0c;
		case 4: return 0x0c;
		case 5: return 0x0b;
		case 6: return 0x0b;
		case 7: return 0x09;
		case 8: return 0x05;
		default: return BLACK_PIXEL;
		};
	};

	// Iterate through the displayBuffer and populate a temp buffer with
	// adjusted values.
	for (int i = 0; i < TAITO_SCREEN_HEIGHT; ++i)
	{
		for (int j = 0; j < TAITO_SCREEN_WIDTH; ++j)
		{
			Uint16* pixel = &this->displayBuffer
							 [i * TAITO_SCREEN_WIDTH
									 + j];
			newBuff[i * TAITO_SCREEN_WIDTH + j] =
					(*pixel == TRANSPARENT_PIXEL)
							? TRANSPARENT_PIXEL
							: getPixelBlurValue(i,
									j,
									pixel);
		}
	}

	// copy new buffer back into old one
	for (int i = 0; i < TAITO_SCREEN_HEIGHT * TAITO_SCREEN_WIDTH; ++i)
		this->displayBuffer[i] = newBuff[i];
}

void TaitoScreen::renderSurface(SDL_Surface* surf)
{
	/* Create a rect that is the same size of the screen for rotations.
	 * x, y, set the position of the top-left corner before the rectangle
	 * is rotated. So it must be set in the right spot so that after
	 * the rotation (which occurs around the center of the rectangle), it
	 * lines up just right with the window.
	 */
	SDL_Rect dest;
	dest.x = (WINDOW_SCALE_FACTOR
				 * (TAITO_SCREEN_HEIGHT - TAITO_SCREEN_WIDTH))
		 / 2;
	dest.y = (WINDOW_SCALE_FACTOR
				 * (TAITO_SCREEN_WIDTH - TAITO_SCREEN_HEIGHT))
		 / 2;
	dest.w = WINDOW_SCALE_FACTOR * TAITO_SCREEN_WIDTH;
	dest.h = WINDOW_SCALE_FACTOR * TAITO_SCREEN_HEIGHT;

	// Create a texture from the passed-in surface
	SDL_Texture* texture = NULL;
	texture = SDL_CreateTextureFromSurface(this->renderer, surf);
	if (texture == NULL)
		std::cout << "Could not create texture. " << SDL_GetError()
			  << std::endl;

	// render it to the screen
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

	// free the texture
	SDL_DestroyTexture(texture);
}

void TaitoScreen::renderFrame()
{
	// apply blur to the to-be rendered frame, then clear the previous
	// render, generate the new one, and flash it to the screen.
	this->applyBlur();
	SDL_RenderClear(this->renderer);
	this->renderSurface(this->colorMasks[this->currColorMask]);
	this->renderSurface(this->gameDisplaySurface);
	SDL_RenderPresent(this->renderer);
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
	for (int i = start_row; i < end_row; ++i)
	{
		for (int j = 0; j < TAITO_SCREEN_WIDTH / 8; ++j)
		{
			pixels = this->taitoVideoBuffer
						 [i * (TAITO_SCREEN_WIDTH / 8)
								 + j];
			for (int k = 0; k < 8; ++k)
			{
				this->displayBuffer[i * TAITO_SCREEN_WIDTH
						    + (j * 8) + k] =
						pixels & bitMasks[k]
								? TRANSPARENT_PIXEL
								: BLACK_PIXEL;
			}
		}
	}
}

int TaitoScreen::handleInput()
{
	// Take control of the keystate and reset/quit mutexes to avoid
	// both threads attempting to read/write at the same time
	pthread_mutex_lock(this->keystateLock);
	pthread_mutex_lock(this->resetQuitLock);

	int quit = 0;
	SDL_Event event;
	/* poll all SDL events until there are no more in the buffer. For
	 * each event, the taitoSCreen will determine whether it cares about
	 * the input, and will take whichever action is appropriate if it does.
	 * If it does not care about a given event, it will be passed on to the
	 * hardware library's update_keystates function to be dealt with there.
	 */
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			*(this->tStruct->quit_flag) = 1;
			quit			    = 1;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode)
			{
			case SDL_SCANCODE_ESCAPE:
				*(this->tStruct->quit_flag) = 1;
				quit			    = 1;
				break;
			default: goto rom_handler;
			};
			break;
rom_handler:
		default: update_keystates(this->tStruct, &event);
		};
	}

	pthread_mutex_unlock(this->resetQuitLock);
	pthread_mutex_unlock(this->keystateLock);
	return quit;
}

TaitoScreen::~TaitoScreen()
{
	delete[] this->displayBuffer;
	for (int i = 0; i < this->numColorMasks; ++i)
		SDL_FreeSurface(this->colorMasks[i]);
	SDL_FreeSurface(this->gameDisplaySurface);
	SDL_Quit();
	delete[] this->colorMasks;
	SDL_Quit();
}
