#include "cpu.h"

#include <assert.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>


int hw_in(const uint8_t* opcode, struct cpu_state* cpu)
{
	assert(opcode[0] == 0b11011011);
	(void) cpu;
	(void) opcode;
#ifdef VERBOSE
	fprintf(stderr, "IN 0x%2.2x (Hardware: basic):\n", opcode[1]);
#endif

	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;
	int is_char_pending = poll(&fds, 1, 0);
	if (is_char_pending == -1)
	{
		perror("Error in polling stdin");
		*cpu->quit_flag = 1;
		return 10;
	}
	else if (is_char_pending && (fds.revents & (POLLERR | POLLHUP | POLLNVAL)))
	{
		*cpu->quit_flag = 1;
		return 10;
	}

	switch(opcode[1])
	{
		case 0x10: //Status register
			
			cpu->a = 0xff;
			//if(is_char_pending) cpu->a |= 1;
			break;
		case 0x11: //Input data
			
			switch(is_char_pending)
			{
				case 0:
					cpu->a = 0x0;
					break;
				default:
					{
						char input = getchar();
						if (input == '\n')
							cpu->a = 0xd;
						else
							cpu->a = input;
						break;
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
	switch(opcode[1])
	{
		case 0x11:
			printf("%c", cpu->a);
			fflush(stdout);
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
