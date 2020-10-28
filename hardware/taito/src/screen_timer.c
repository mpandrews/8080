#include <errno.h>
#include <stdint.h>
#include <time.h>

/* The Taito screen has 262 'lines'.  These do not translate 1:1 into
 * real TV scanlines (there are 525 of those,) but that's fine.  We're
 * only interested in the timing, here.
 *
 * NTSC color screens run at 59.94Hz.  That means that each frame
 * will take 16,183,350.02 nanoseconds.  Divided by our 262 lines,
 * that gives us a time of 63,676.908 nanoseconds per line.
 * Now the interrupt timing.
 *
 * According to the MAME people (in the comments of mw8080bw.cpp),
 * The first interrupt (0xcf) is sent after line 96.  The second interrupt
 * is sent after line 224.  That leaves the 38 line VBLANK period counting
 * as effectively part of the the first interval.  So we have 134 line
 * intervals for the top half (96 + 38) and 128 for the second.
 *
 * This works out to roughly 8 milliseconds for each half, but there's a
 * ~5% difference, which seems too large to handwave, so we'll use
 * nanosleep() to at least aim for the exact target time.
 */
#define NS_PER_LINE 63677

#define TOP_LINES    134
#define BOTTOM_LINES 128

void screen_timer()
{
	static uint8_t is_bottom;
	static struct timespec target;

	// Initialize the timer on first call.
	if (target.tv_sec == 0 && target.tv_nsec == 0)
	{ clock_gettime(CLOCK_MONOTONIC, &target); }

	target.tv_nsec += is_bottom ? BOTTOM_LINES * NS_PER_LINE
				    : TOP_LINES * NS_PER_LINE;

	if (target.tv_nsec > 1000000000)
	{
		++target.tv_sec;
		target.tv_nsec -= 1000000000;
	}
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, NULL)
			&& errno == EINTR)
		;
	is_bottom = !is_bottom;
}
