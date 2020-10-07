#include "../include/cpu.h"
#include "../include/cycle_timer.h"
#include "../include/opcode_array.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

//Diagnostic print function to dump the CPU's state to stderr.
static inline void print_registers(const struct cpu_state* cpu)
{
	uint8_t flags[9];
	uint8_t* f = flags;
	for (uint8_t mask = 0x80; mask; mask >>= 1, ++f)
		*f = mask & LOW_REG8(cpu->psw) ? '1' : '0';
	*f = 0;
	fprintf(stderr,
			"\tPC:  0x%4.4x -> 0x%2.2x\n"
			"\tBC:  0x%4.4x -> 0x%2.2x\n"
			"\tDE:  0x%4.4x -> 0x%2.2x\n"
			"\tHL:  0x%4.4x -> 0x%2.2x\n"
			"\tPSW: 0x%4.4x\n"
			"\tSP:  0x%4.4x -> 0x%2.2x\n"
			"\tAddress Bus: 0x%4.4x\n"
			"\tData Bus: 0x%2.2x\n"
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
			*cpu->address_bus,
			*cpu->data_bus,
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
			.interrupt_buffer = res->interrupt_buffer,
			.address_bus	  = res->address_bus,
			.data_bus	  = res->data_bus};

	uint8_t opcode;

	// We can remove this assignment if we want to force the user
	// to hardware reset on CPU boot.
	cpu.reset_flag = 1;
	for (;;)
	{
		// If the reset flag is set, reset and clear it.
		// This is the hardware reset.
		if (cpu.reset_flag)
		{
			cpu.pc	       = 0;
			cpu.reset_flag = 0;
			cpu.halt_flag  = 0;
			// Resetting is a 3-cycle operation.
			cycle_wait(3);
			continue;
		}
		switch (cpu.interrupt_enable_flag)
		{
		/* If the interrupt flag is currently enabled,
		 * then we we our opcode from the interrupt buffer
		 * if there's anything there, otherwise we
		 * fetch normally.
		 *
		 * Note that we also clear the halt flag, if it was
		 * set.  That way the interrupt will actually execute
		 * as expected.  Not that if the halt flag
		 * is set and interrupts are disabled, the CPU
		 * can only resume execution via hardware reset.
		 */
		case 1: // Interrupt flag is enabled.
			pthread_mutex_lock(cpu.int_lock);
			if (cpu.interrupt_buffer)
			{
				opcode		      = *cpu.interrupt_buffer;
				*cpu.interrupt_buffer = 0;
				cpu.halt_flag	      = 0;
				cpu.interrupt_enable_flag = 0;
				pthread_cond_signal(cpu.int_cond);
			}
			else
				opcode = cpu.memory[cpu.pc];
			pthread_mutex_unlock(cpu.int_lock);
			break;
		/* If the interrupt flag is pending enablement,
		 * which will happen if that last opcode we
		 * ran was EI, then we decrement it and fetch
		 * normally.  That way, when the next
		 * opcode gets its turn, the flag will be set.
		 * If the flag is clear, we just
		 * fetch normally.
		 */
		case 2: // Interrupt flag is pending enable.
			--cpu.interrupt_enable_flag;
			// FALLTHRU
		default: opcode = cpu.memory[cpu.pc];
		}
		/* If we pulled an interrupt, then the halt flag
		 * will have been cleared, so if it's still set,
		 * then we need to spinlock.  If interrupts are enabled,
		 * then we'll spin for 10 cycles, in case someone wants
		 * to throw something on our buffer.  If they're not,
		 * the only way out is via hardware reset, so we'll wait
		 * a full timechunk to make sure the keyboard has been
		 * polled again before we go through the loop.
		 *
		 * We may consider switching this out for a more elaborate
		 * system of locks and pthread conds to avoid spinlocking,
		 * but it's also not the end of the world if the (real)
		 * CPU spinlocks when the (fake) CPU wedges itself.
		 *
		 * We can chalk it up to 'technically, if unhelpfully,
		 * faithful.'
		 *
		 * One possibility for improvement if we implement a global
		 * keyboard-state struct is that we can put a lock around it
		 * and add a condition flag for hardware reset, so that the CPU
		 * can simply block while waiting for it, if we're in the
		 * full wedge state.
		 */
		switch (cpu.halt_flag)
		{
		case 0: cycle_wait(opcodes[opcode](opcode, &cpu));
#ifdef VERBOSE
			print_registers(&cpu);
#endif
			break;
		default:
			if (cpu.interrupt_enable_flag)
				// Strictly speaking, halt should
				// spin for 1 cycle, but given
				// our timechunking, that will just
				// burn more (real) CPU without
				// increasing responsiveness.
				cycle_wait(10);
			else
				// Full-wedge.
				cycle_wait(CYCLE_CHUNK);
		}
	}
	return NULL;
}
