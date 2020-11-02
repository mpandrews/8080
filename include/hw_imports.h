#ifndef HW_IMPORTS_H
#define HW_IMPORTS_H

#include "cpu.h"

#include <stdlib.h>

/* These are the five functions that every hardware library is obligated
 * to provide.  Failure to provide any of them is a fatal error.
 */

__attribute__((weak)) int hw_in(const uint8_t* opcode, struct cpu_state*);

__attribute__((weak)) int hw_out(const uint8_t* opcode, struct cpu_state* cpu);

__attribute__((weak)) int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*));

__attribute__((weak)) void* hw_init_struct(struct system_resources* res);

__attribute__((weak)) void hw_destroy_struct(void* hw_struct);

__attribute__((weak)) void* front_end(void*);

#endif
