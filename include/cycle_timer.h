#ifndef CYCLE_TIMER
#define CYCLE_TIMER

// Because we're using the monotonic high-resolution clock, which
// measures time in nanoseconds, we define one clock pulse of the
// CPU in number of nanoseconds elapsed.  At 2MHz, one clock cycle
// is .5 microseconds, which is 500 nanoseconds.
#define CYCLE_TIME (500l)

// Because there's considerable overhead in calling sleep,
// we don't want to try to sleep after every opcode.  Instead, we'll
// keep track of how many system cycles *should* have elapsed, and just
// rest in chunks.  A chunk here is how many clock cycles we're allowing
// to elapse before we force a sleep.
#define CYCLE_CHUNK (512)

/*
 * cycle_wait takes as its only argument the number of cycles to be waited.
 * It keeps a static internal count of the number of cycles that have elapsed,
 * and it exceeds its target, it will sleep until the correct wall time.
 */
void cycle_wait(int cycles);

#endif
