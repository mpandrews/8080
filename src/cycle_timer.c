#include "cycle_timer.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifndef BENCHMARK
void cycle_wait(int cycles)
{
	static int count;
	static struct timespec target;
	// If we haven't yet initialized the timer, we need to.
	if (target.tv_sec == 0 && target.tv_nsec == 0)
	{ clock_gettime(CLOCK_MONOTONIC, &target); }

	count += cycles;
	// If a chunk's worth of cycles have elapsed, it's time to sleep.
	if (count >= CYCLE_CHUNK)
	{
		// Adjust the target time upward by the amount of time
		// the elapsed cycle count should have taken.
		target.tv_nsec += count * CYCLE_TIME;
#	ifdef VERBOSE
		fprintf(stderr,
				"t sec: %ld\nt nsec: %ld\n",
				target.tv_sec,
				target.tv_nsec);
#	endif
		if (target.tv_nsec > 999999999)
		{
			target.tv_nsec -= 999999999;
			++target.tv_sec;
		}
		count = 0;
		// Sleep until we hit the target time, if and only if
		// that time is in the future.  clock_nanosleep()
		// will return immediately without suspending execution
		// if the requested time is in the past.  That should
		// let us catch up if we fall behind: the return is very quick.
		while (clock_nanosleep(CLOCK_MONOTONIC,
				       TIMER_ABSTIME,
				       &target,
				       NULL)
				&& errno == EINTR)
			;
	}
}
#else // BENCHMARK is defined
/*
 * This version of cycle_wait() just lets the program run unthrottled
 * and calculates its effective spead in hertz.  Note that it only looks at CPU
 * time for the thread, however, so elides real delays.
 */
void cycle_wait(int cycles)
{
	static size_t count;
	static clock_t last;
	if (!last) { last = clock(); }
	count += cycles;
	if (count >= (1 << 30))
	{
		clock_t new = clock();
		printf("%lf cycles per second.\n",
				(double) count
						/ ((double) (new - last)
								/ CLOCKS_PER_SEC));
		last  = new;
		count = 0;
	}
}
#endif
