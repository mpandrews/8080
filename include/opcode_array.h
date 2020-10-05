#ifndef OPCODE_ARRAY
#define OPCODE_ARRAY

#include "cpu.h"

/* This simply defines the opcode array itself.  It should be included
 * in any .c file that wants to call opcodes, so that it knows about the
 * array.
 */

extern int (*const opcodes[256])(uint8_t opcode, struct cpu_state* cpu);

#endif
