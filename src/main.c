#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/cpu.h"
#include "../include/opcode_array.h"

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

}
