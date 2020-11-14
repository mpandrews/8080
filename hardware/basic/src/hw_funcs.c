#include "cpu.h"

#include <assert.h>
#include <curses.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "IN 0x%2.2x (Hardware: basic)\n", opcode[1]);
#endif

	switch (opcode[1])
	{
	case 0x10: // Status register
		/*
		 * The BASIC program will regularly poll this port to see if
		 * the serial board buffers are ready for read or write.
		 * By sending back an all-one byte, we tell it that it can
		 * always safely send or receive, which is true, since there
		 * isn't actually a serial port involved and there's no
		 * realtime timing issue here.
		 */
		cpu->a = 0xff;
		break;
	case 0x11: // Input data
		// clang-format off
	// If we don't ignore clang-formatting here, our braces get unindented.
		{	   // New scope to allow a variable declaration.
			int input = getch();
			switch (input)
			{
			// getch() will return ERR if nothing is pending.  
			// We can safely return a NULL byte, and BASIC will
			// simply ignore it.
			case ERR: cpu->a = 0; break;
			// By default, Linux uses \n to mean new line and 
			// carriage return. We actually only need carriage 
			// return, so we replace it.
			case '\n': cpu->a = '\r'; break;
			case 0x04:			       // FALLTHRU (^D)
			case 0x1c:			       // FALLTHRU (^\)
			case 0x1b: *cpu->quit_flag = 1; break; // ESC
			default: cpu->a = input; break;
			}
		}
		// clang-format on
	}

	return 10;
}

int hw_out(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11010011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "OUT 0x%2.2x (Hardware: basic):\n", opcode[1]);
#endif
	switch (opcode[1])
	{
	case 0x11:
		// For reasons unknown to me, in certain specific circumstances
		// BASIC sets the highest bit when printing.  This seems to
		// happen specifically to the first letter of each word
		// when using LIST.  Clearing the high bit before
		// printing gets us the right output.
		switch (cpu->a & ~0x80)
		{
		// Carriage return needs to be skipped, or the line
		// will blank itself on CRLF.
		case '\r': break;
		case '\a': beep(); break;
		case '\f': erase(); break;
		default: echochar(cpu->a & ~0x80);
		}
	}
	return 10;
}

// Interrupt Hook
int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*))
{
	return op_func(opcode, cpu);
}

// Init Struct
void* hw_init_struct()
{
	initscr();
	raw(); // Raw keyboard input: shell will pass through Ctrl-combinations.
	nonl();
	noecho(); // Do not echo the user's typing: BASIC already does so.
	meta(stdscr, FALSE);	// Set 7-bit ASCII.
	nodelay(stdscr, TRUE);	// Don't insert an artifical delay for combos.
	scrollok(stdscr, TRUE); // Allow the 'screen' to 'scroll'.
	immedok(stdscr, TRUE);	// Refresh after every character write.
	return NULL;
}

// Destroy Struct
void hw_destroy_struct(void* hw_struct)
{
	(void) hw_struct;
	endwin();
	return;
}
