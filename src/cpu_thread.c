#include "cpu.h"
#include "cycle_timer.h"
#include "hw_func_pointers.h"
#include "opcode_array.h"
#include "opcode_size.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Diagnostic print function to dump the CPU's state to stderr.
static inline void print_registers(const struct cpu_state* cpu)
{
	uint8_t flags[9];
	uint8_t* f = flags;
	for (uint8_t mask = 0x80; mask; mask >>= 1, ++f)
		*f = mask & cpu->flags ? '1' : '0';
	*f = 0;
	fprintf(stderr,
			"\tPC:  0x%4.4x -> 0x%2.2x\n"
			"\tBC:  0x%4.4x -> 0x%2.2x\n"
			"\tDE:  0x%4.4x -> 0x%2.2x\n"
			"\tHL:  0x%4.4x -> 0x%2.2x\n"
			"\tPSW: 0x%4.4x\n"
			"\tSP:  0x%4.4x -> 0x%2.2x\n"
			"\tFlags: %s\n"
			"\t       SZ-A-P-C\n",
			cpu->pc,
			cpu->memory[cpu->pc],
			cpu->bc,
			cpu->memory[cpu->bc],
			cpu->de,
			cpu->memory[cpu->de],
			cpu->hl,
			cpu->memory[cpu->hl],
			cpu->psw,
			cpu->sp,
			cpu->memory[cpu->sp],
			flags);
}

void* cpu_thread_routine(void* resources)
{
	// A shallow copy is fine, since we actually DO want
	// to simply duplicate the pointers verbatim.
	// struct cpu_state cpu;
	struct system_resources* res = (struct system_resources*) resources;

	struct cpu_state cpu = {.memory	  = res->memory,
			.int_lock	  = res->interrupt_lock,
			.int_cond	  = res->interrupt_cond,
			.reset_quit_lock  = res->reset_quit_lock,
			.reset_flag	  = res->reset_flag,
			.quit_flag	  = res->quit_flag,
			.interrupt_buffer = res->interrupt_buffer,
			.hw_struct	  = res->hw_struct,
			.rom_mask	  = res->rom_mask,
			.mask_shift	  = res->mask_shift};

	// We can remove this assignment if we want to force the user
	// to hardware reset on CPU boot.
	cpu.pc = 0;
	const uint8_t* opcode;
	for (;;)
	{
		switch (cpu.interrupt_enable_flag << 1 | cpu.halt_flag)
		{
		case 4: // Interrupt pending, not halted.
			--cpu.interrupt_enable_flag;
			// FALLTHRU
		case 0: // Interrupt disabled, not halted.
			goto normal_execution;
		case 5: // Interrupt pending, halted.
			--cpu.interrupt_enable_flag;
			// FALLTHRU
		case 3: // Interrupt enabled, halted.
			pthread_mutex_lock(cpu.int_lock);
			if (cpu.interrupt_buffer) goto interrupt_execution;
			pthread_mutex_unlock(cpu.int_lock);
			if (cycle_wait(10, &cpu)) return 0;
			break;
		case 2: // Interrupt enabled, not halted.
			pthread_mutex_lock(cpu.int_lock);
			if (*cpu.interrupt_buffer) goto interrupt_execution;
			pthread_mutex_unlock(cpu.int_lock);
			goto normal_execution;
			break;
		case 1: // Interrupt disabled, halted.
			if (cycle_wait(CYCLE_CHUNK, &cpu)) return 0;
			break;
normal_execution:
			opcode = cpu.memory + cpu.pc;
#ifdef VERBOSE
			fprintf(stderr, "0x%4.4x: ", cpu.pc);
#endif
			cpu.pc += get_opcode_size(opcode[0]);
			// cycle_wait returns 1 if a quit event is pending.
			if (cycle_wait(opcodes[opcode[0]](opcode, &cpu), &cpu))
				return 0;
#ifdef VERBOSE
			print_registers(&cpu);
#endif
			break;
interrupt_execution:
			// The 8080 only supports single-byte opcodes
			// as interrupts.  So if we get a multi-byte,
			// we'll just clear it and move on.
			if (get_opcode_size(*cpu.interrupt_buffer) > 1)
			{
				*cpu.interrupt_buffer = 0;
				pthread_mutex_unlock(cpu.int_lock);
				pthread_cond_signal(cpu.int_cond);
				break;
			}
			cpu.halt_flag		  = 0;
			cpu.interrupt_enable_flag = 0;
			// Ignore return value: we don't advance
			// PC for interrupts, though they can jump us.
#ifdef VERBOSE
			fprintf(stderr, "INTRPT: ");
#endif
			if (cycle_wait(interrupt_hook(cpu.interrupt_buffer,
						       &cpu,
						       opcodes[*cpu.interrupt_buffer]),
					    &cpu))
				return 0;
			*cpu.interrupt_buffer = 0;
			pthread_mutex_unlock(cpu.int_lock);
			pthread_cond_signal(cpu.int_cond);
#ifdef VERBOSE
			print_registers(&cpu);
#endif
			break;
		}
	}
	return NULL;
}
