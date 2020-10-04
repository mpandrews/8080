#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/cpu.h"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "USAGE:\n\t%s <FILENAME>\n", *argv);
		exit(1);
	}

	//Allocate the memory space for the CPU.
	uint8_t* memory_space = malloc(MAX_MEMORY);
	if (!memory_space)
	{
		perror("Malloc error creating memory for 8080");
		exit(1);
	}
	
	//Open the file argument.
	int file = open(argv[1], O_RDONLY);
	if (file == -1)
	{
		perror("Error opening file for reading");
		exit(1);
	}

	//Block to avoid these temporary variables taking up stack space.
	//We'll then read the file into the memory buffer, and then close it.

	{
		ssize_t last_read;
		uint8_t* buffer = memory_space;
		size_t remaining_space = MAX_MEMORY;

		do
		{
			last_read = read(file, buffer, remaining_space);
			if (last_read == -1)
			{
				if (errno == EINTR)
					continue;
				perror("Error reading input file");
				exit(1);
			}
			buffer += last_read;
			remaining_space -= last_read;
		}while(remaining_space && last_read);
		close(file);
	}

	pthread_cond_t interrupt_condition;
	pthread_cond_init(&interrupt_condition, NULL);
	pthread_mutex_t interrupt_lock;
	pthread_mutex_init(&interrupt_lock, NULL);
	uint8_t data_bus;
	uint16_t address_bus;
	uint8_t interrupt_buffer;

	struct system_resources res = {
		.interrupt_cond = &interrupt_condition,
		.interrupt_lock = &interrupt_lock,
		.memory = memory_space,
		.interrupt_buffer = & interrupt_buffer,
		.data_bus = &data_bus,
		.address_bus = &address_bus
	};

	pthread_t cpu_t;
	pthread_create(&cpu_t, NULL, cpu_thread, (void*) &res);

	pthread_join(cpu_t, NULL);

	exit(0);
}
