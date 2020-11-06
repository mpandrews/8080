#ifndef HW_FUNC_POINTERS_H
#define HW_FUNC_POINTERS_H

#include "cpu.h"

/* These function pointers represent four of the six functions which the main
 * program will look for when loading in a hardware library at runtime.  They
 * will be populated using dlsym().  (The other two functions are definitions
 * for the opcodes IN and OUT, for which pointers exist in the opcode array.
 * Failure to define these in a hardware library is an irrecoverable error. */

/* interrupt_hook will be run whenever an interrupt is actually executed.  That
 * is to say, if enable_interrupt_flag is equal to 1 during instruction fetch,
 * and the interrupt_buffer is populated, actual execution will be deferred to
 * this function.  The function takes a pointer to the opcode, a pointer to the
 * cpu struct, and a pointer to the actual opcode function itself.  (Since it
 * can't see the opcode array.)  It may do whatever it needs to: in the case of
 * the taito games, we use this opportunity to refresh the video buffer.  Do
 * whatever you need to do.
 */
extern int (*interrupt_hook)(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*));

/* It is expected that any hardware library will have some data it needs
 * to lug around.  You can initialize it here.  Whatever void* you
 * return will be included as the rom_struct member of the cpu_state struct,
 * so you will have access to it throughout execution, though you will need
 * to cast it to access it.
 */
extern void* (*hw_init_struct)(struct system_resources*);

/* Any cleanup your hardware data struct needs to do should happen here.
 */
extern void (*hw_destroy_struct)(void*);

/* OPTIONAL if front_end is defined, it will be executed in a separat thread
 * running parallel to the main cpu thread.  If you want video, or a GUI,
 * or anything else, really: this is the place.  Do whatever you want to in
 * here, but you will be responsible for providing some way of exiting:
 * the main thread will not forcibly end your thread.  The argument
 * passed in will be your hardware struct.
 */
extern void* (*front_end)(void*);

#endif
