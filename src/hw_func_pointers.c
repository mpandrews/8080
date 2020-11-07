// See associated .h for documentation.

#include "cpu.h"

int (*interrupt_hook)(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*));

void* (*hw_init_struct)(struct system_resources*);

void (*hw_destroy_struct)(void*);

void* (*front_end)(void*);
