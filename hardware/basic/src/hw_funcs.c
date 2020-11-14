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

		cpu->a = 0xff;
		break;
	case 0x11: // Input data
	{
		int input = getch();
		switch (input)
		{
		case ERR: cpu->a = 0; break;
		case '\n': cpu->a = 0xd; break;
		default: cpu->a = input; break;
		}
	}
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
		switch (cpu->a)
		{
		// Carriage return needs to be skipped, or the line
		// will blank itself on CRLF.
		case '\r': break;
		case '\a': beep(); break;
		case '\f': erase(); break;
		default: echochar(cpu->a);
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
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	immedok(stdscr, TRUE);
	return NULL;
}

// Destroy Struct
void hw_destroy_struct(void* hw_struct)
{
	(void) hw_struct;
	endwin();
	return;
}
