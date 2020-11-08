#include "cpu.h"
#include "hw_func_pointers.h"
#include "opcode_array.h"
#include "opcode_decls.h"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static inline void parse_arguments(
		int argc, char** argv, char* rom_name, char* hw_lib_name);

static inline void find_hw_funcs(void* hw_lib_handle, char* hw_lib_name);

static inline void read_rom_mask(
		char* rom_name, uint8_t* mask_shift, uint8_t** rom_mask);

int main(int argc, char** argv)
{
	void* hw_lib_handle;
	(void) hw_lib_handle;
	char hw_lib_name[20] = {0};
	char rom_name[50]    = {0};
	/* Arbitrary block to keep the stack clean-ish.
	 * Parse the command-line options.
	 */

	parse_arguments(argc, argv, rom_name, hw_lib_name);

	// Allocate the memory space for the CPU.
	uint8_t* memory_space = malloc(MAX_MEMORY);
	if (!memory_space)
	{
		perror("Malloc error creating memory for 8080");
		exit(1);
	}

	// Open the file argument.
	int file = open(rom_name, O_RDONLY);
	if (file == -1)
	{
		perror("Error opening file for reading");
		exit(1);
	}

	hw_lib_handle = dlopen(hw_lib_name, RTLD_NOW);
	// dlopen returns NULL on failure.
	if (!hw_lib_handle)
	{
		fprintf(stderr,
				"Error opening hardware library:\n\t%s\n",
				dlerror());
		exit(1);
	}

	find_hw_funcs(hw_lib_handle, hw_lib_name);

	// Block to avoid these temporary variables taking up stack space.
	// We'll then read the file into the memory buffer, and then close it.

	{
		ssize_t last_read;
		uint8_t* buffer	       = memory_space;
		size_t remaining_space = MAX_MEMORY;

		do
		{
			last_read = read(file, buffer, remaining_space);
			if (last_read == -1)
			{
				if (errno == EINTR) continue;
				perror("Error reading input file");
				exit(1);
			}
			buffer += last_read;
			remaining_space -= last_read;
		} while (remaining_space && last_read);
		close(file);
	}

	uint8_t mask_shift;
	uint8_t* rom_mask;

	read_rom_mask(rom_name, &mask_shift, &rom_mask);

	pthread_cond_t interrupt_condition;
	pthread_cond_init(&interrupt_condition, NULL);
	pthread_mutex_t interrupt_lock;
	pthread_mutex_init(&interrupt_lock, NULL);
	pthread_mutex_t reset_quit_lock;
	pthread_mutex_init(&reset_quit_lock, NULL);
	uint8_t reset_flag	 = 0;
	uint8_t quit_flag	 = 0;
	uint8_t interrupt_buffer = 0;

	struct system_resources res = {.interrupt_cond = &interrupt_condition,
			.interrupt_lock		       = &interrupt_lock,
			.reset_quit_lock	       = &reset_quit_lock,
			.memory			       = memory_space,
			.interrupt_buffer	       = &interrupt_buffer,
			.reset_flag		       = &reset_flag,
			.quit_flag		       = &quit_flag,
			.rom_mask		       = rom_mask,
			.mask_shift		       = mask_shift};

	res.hw_struct = hw_init_struct(&res);
	pthread_t front_end_thread;

	// We check to see if the hardware library defines a front_end function:
	// if we do, we spin it up in a second thread.  If not, there's
	// nothing to do here.
	front_end = dlsym(hw_lib_handle, "front_end");
	if (front_end)
	{
		pthread_create(&front_end_thread,
				NULL,
				front_end,
				(void*) res.hw_struct);
	}

	// Run the CPU routine in this thread.
	cpu_thread_routine(&res);

	// If we have a front_end, then we'll cancel and join it after
	// the cpu routine routines.
	if (front_end) { pthread_join(front_end_thread, NULL); }
	// Cleanup.
	free(memory_space);
	free(rom_mask);
	hw_destroy_struct(res.hw_struct);
	dlclose(hw_lib_handle);
	exit(0);
}

const char* const USAGE = "Usage: %s\n"
			  "\t{-r ROM_FILE|--rom ROM_FILE}\n"
			  "\t [--hw HARDWARE_NAME|--hardware HARDWARE_NAME]\n"
			  "\t[-h|--help]\n\n"
			  "Options:\n"
			  "\t-r, --rom\n"
			  "\t\tThe name of the 8080 rom file to execute.\n"
			  "\t\tRequired.\n"
			  "\t--hw --hardware\n"
			  "\t\tThe name of the hardware library to use.\n"
			  "\t\tAvailable options are:"
			  "'si',"
			  "'none',"
			  "'cpudiag'.\n"
			  "\t\tDefaults to 'none' if not specified.\n"
			  "\t-h, --help\n"
			  "\t\tPrint this message.\n";

void parse_arguments(int argc, char** argv, char* rom_name, char* hw_lib_name)
{
	char rom_found		   = 0;
	char hw_found		   = 0;
	int opt_return		   = 0;
	int option_index	   = 0;
	struct option long_opts[5] = {{"rom", required_argument, 0, 'r'},
			{"hardware", required_argument, 0, 'H'},
			{"hw", required_argument, 0, 'H'},
			{"help", no_argument, 0, 'h'},
			{0}};
	while ((opt_return = getopt_long(
				argc, argv, "r:h", long_opts, &option_index))
			!= -1)
	{
		switch (opt_return)
		{
		case 'r':
			if (rom_found)
			{
				fprintf(stderr,
						"Only one ROM file can be "
						"specified.\n");
				fprintf(stderr, USAGE, *argv);
				exit(1);
			}
			rom_found   = 1;
			rom_name[0] = 0;
			strncpy(rom_name, optarg, 49);
			break;
		case 'h': printf(USAGE, *argv); exit(0);
		case 'H':
			if (hw_found)
			{
				fprintf(stderr,
						"Only one hardware set can be "
						"specified.\n");
				fprintf(stderr, USAGE, *argv);
				exit(1);
			}
			hw_found       = 1;
			hw_lib_name[0] = 0;
			strcpy(hw_lib_name, "hw/lib");
			strncat(hw_lib_name, optarg, 10);
			strcat(hw_lib_name, ".so");
			break;
		case '?': // FALLTHRU
		default: fprintf(stderr, USAGE, *argv); exit(1);
		}
	}
	if (!rom_found)
	{
		fprintf(stderr, "No ROM file specified!\n");
		fprintf(stderr, USAGE, *argv);
		exit(1);
	}
	if (!hw_found)
	{
		hw_lib_name[0] = 0;
		strcpy(hw_lib_name, "hw/libnone.so");
	}
}

void find_hw_funcs(void* hw_lib_handle, char* hw_lib_name)
{
	// Assign the OUT opcode function defined in the HW lib.
	opcodes[0xd3] = dlsym(hw_lib_handle, "hw_out");
	if (!opcodes[0xd3])
	{
		fprintf(stderr,
				"Error finding function 'hw_out' in hardware"
				" library %s:\n\t%s\n",
				hw_lib_name,
				dlerror());
		exit(1);
	}
	// Assign the IN opcode function.
	opcodes[0xdb] = dlsym(hw_lib_handle, "hw_in");
	if (!opcodes[0xdb])
	{
		fprintf(stderr,
				"Error finding function 'hw_in' in hardware"
				" library %s:\n\t%s\n",
				hw_lib_name,
				dlerror());
		exit(1);
	}

	// Hardware-defined function to initialize whatever data parcel
	// the hardware wants the CPU to carry around: returns a void*,
	// which the HW-specific functions can then cast to whatever it really
	// is.
	hw_init_struct = dlsym(hw_lib_handle, "hw_init_struct");
	if (!hw_init_struct)
	{
		fprintf(stderr,
				"Error finding function 'hw_init_struct' in "
				"hardware"
				" library %s:\n\t%s\n",
				hw_lib_name,
				dlerror());
		exit(1);
	}
	// HW-defined function to clean up its struct.
	hw_destroy_struct = dlsym(hw_lib_handle, "hw_destroy_struct");
	if (!hw_destroy_struct)
	{
		fprintf(stderr,
				"Error finding function 'hw_destroy_struct' in "
				"hardware"
				" library %s:\n\t%s\n",
				hw_lib_name,
				dlerror());
		exit(1);
	}

	interrupt_hook = dlsym(hw_lib_handle, "hw_interrupt_hook");
	if (!hw_destroy_struct)
	{
		fprintf(stderr,
				"Error finding function 'hw_interrupt_hook' in "
				"hardware"
				" library %s:\n\t%s\n",
				hw_lib_name,
				dlerror());
		exit(1);
	}

	front_end = dlsym(hw_lib_handle, "front_end");
}

void read_rom_mask(char* rom_name, uint8_t* mask_shift, uint8_t** rom_mask)
{
	char mask_file_name[55] = {0};
	strncpy(mask_file_name, rom_name, 50);
	strcat(mask_file_name, ".mask");
	int fd = open(mask_file_name, O_RDONLY);
	if (fd == -1)
	{
#ifdef VERBOSE
		perror("Could not open ROM mask file, emulated memory"
		       " will be vulnerable to corruption"
		       " by ROM");
#endif
		*mask_shift = 16;
		*rom_mask   = malloc(1);
		**rom_mask  = 0;
		return;
	}
	ssize_t last_read;
	size_t remaining_space;
	if (!read(fd, mask_shift, 1))
	{
		perror("Error reading mask file");
		exit(1);
	}
	else if (*mask_shift > 15)
	{
		fprintf(stderr, "Invalid mask file!");
		exit(1);
	}
	*rom_mask = malloc(MAX_MEMORY >> *mask_shift);
	memset(*rom_mask, 0, MAX_MEMORY >> *mask_shift);
	remaining_space = MAX_MEMORY >> *mask_shift;
	uint8_t* buffer = *rom_mask;
	do
	{
		last_read = read(fd, buffer, remaining_space);
		if (last_read == -1)
		{
			if (errno == EINTR) continue;
			perror("Error reading input file");
			exit(1);
		}
		buffer += last_read;
		remaining_space -= last_read;
	} while (remaining_space && last_read);
	close(fd);
}
