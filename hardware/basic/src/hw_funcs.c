#include "cpu.h"

#include <assert.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>

extern void cycle_wait(int);

// IN
int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "IN (Hardware: basic)\n");
#endif
	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;

	switch(opcode[1])
	{
		case 0x10: //Status port?
			cpu->a = 1 | 2 | 4;
			break;
		case 0x11:
			switch(poll(&fds, 1, 0))
			{
				case -1:
					perror("Error in polling STDIN");
					*cpu->quit_flag = 1;
					break;
				case 0:
					cpu->a = 0x0;
					break;
				default:
					if (fds.revents & (POLLERR | POLLHUP | POLLNVAL))
					{
						*cpu->quit_flag = 1;
						break;
					}
					cpu->a = getchar();
					break;
			}
	}

//	printf("IN: %x\n PORT: %x\n", cpu->a, opcode[1]);
	return 10;
}

// OUT
int hw_out(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11010011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "OUT (Hardware: basic)\n");
#endif
	switch(opcode[1])
	{
		case 0x11:
			printf("%c", cpu->a);
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
	return NULL; 
}

// Destroy Struct
void hw_destroy_struct(void* hw_struct)
{
	(void) hw_struct;
	return;
}
