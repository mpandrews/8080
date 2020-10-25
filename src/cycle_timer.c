#include "cycle_timer.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifdef UNTHROTTLED
void cycle_wait(int cycles) { (void) cycles; }
#else // ifndef UNTHROTTLED
void cycle_wait(int cycles)
{
	static int count;
	static struct timespec target;
#	ifdef BENCHMARK
	static int bench_count;
	static struct timespec bench_last;
#	endif
	// If we haven't yet initialized the timer, we need to.
	if (target.tv_sec == 0 && target.tv_nsec == 0)
	{ clock_gettime(CLOCK_MONOTONIC, &target); }
	count += cycles;
#	ifdef BENCHMARK
	if (bench_last.tv_sec == 0 && bench_last.tv_nsec == 0)
	{ clock_gettime(CLOCK_MONOTONIC, &bench_last); }
	bench_count += cycles;
#	endif
	// If a chunk's worth of cycles have elapsed, it's time to sleep.
	if (count >= CYCLE_CHUNK)
	{
		// Adjust the target time upward by the amount of time
		// the elapsed cycle count should have taken.
		target.tv_nsec += count * CYCLE_TIME;
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
#	ifdef BENCHMARK
	if (bench_count >= BENCH_INTERVAL) //~8 million unless overridden
	{
		struct timespec new;
		clock_gettime(CLOCK_MONOTONIC, &new);
		double elapsed = new.tv_sec;
		elapsed += (double) new.tv_nsec / 1000000000.0;
		elapsed -= bench_last.tv_sec;
		elapsed -= (double) bench_last.tv_nsec / 1000000000.0;
		fprintf(stderr,
				"Effective speed: %lfMHz\n",
				(double) bench_count / elapsed / 1000000.0);
		bench_count = 0;
		bench_last  = new;
	}
#	endif // BENCHMARK
}
#endif	       // ifndef UNTHROTTLED
