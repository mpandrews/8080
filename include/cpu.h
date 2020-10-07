#ifndef CPU
#define CPU

#include <pthread.h>
#include <stdint.h>

// The maximum memory addressable by the CPU is 2^16 bytes.
#define MAX_MEMORY (1 << 16)

/* Macros for pulling an 8-bit subregister out of a full 16-bit register.
 * Note that this is little-endian: the high register is in the second
 * byte, the byte with the higher memory address.  This accords with
 * how push/pop operations work.
 *
 * This can get a touch confusing when it comes to the naming:
 * as an example, take the BC register pair.  B is the high register, and
 * C is the low register.  Thus if BC is at 0x400, then C is at 0x400 and B
 * is at Ox401.  HIGH_REG8(cpu.bc) will evaluate to the byte at 0x401, which
 * is B.
 *
 * Note that the diagrams in the manual tend not to show it this way when
 * depicting registers.  Be careful.
 */
#define LOW_REG8(REG16)	 (*((uint8_t*) &(REG16)))
#define HIGH_REG8(REG16) (*(((uint8_t*) &(REG16)) + 1))

/* Defines for accessing the flags.  These defines are designed
 * to be used on the 16-bit psw register member of the CPU struct.
 * You do not need to extract just that byte, although you can.
 * Because the flag byte is the low subregister, the carry flag is
 * bit 0 of the combined larger register.
 *
 * Usage examples:
 *
 * 	Set carry flag:
 * 		cpu.psw |= CARRY_FLAG;
 * 	Here, we've set the carry flag by ORing the register with 1:
 * 	0000 0001 0000 0000.  Recall that this is all little-endian,
 * 	so the first of the two bytes is the less significant.  The actual
 * 	defined constant is an int, so there will be an additional two
 * 	bytes of trailing zeroes that we don't care about.
 *
 * 	Clear zero flag:
 * 		cpu.psw &= ~ZERO_FLAG;
 * 	Here, we've cleared the zero flag by ANDing the register with the
 * 	inverse of the zero flag: 1011 1111 1111 1111.
 *
 * 	Check sign flag:
 * 		if (cpu.psw & SIGN_FLAG) ...
 */

#define CARRY_FLAG     (1u)	 // Bit 0.
#define PARITY_FLAG    (1u << 2) // Bit 2.
#define AUX_CARRY_FLAG (1u << 4)
#define ZERO_FLAG      (1u << 6)
#define SIGN_FLAG      (1u << 7)

struct cpu_state
{

	/* For interrupts, rather than use the actual 'pins',
	 * we can just use a mutex and an additional little buffer
	 * in which pending interrupts can be stored.
	 * We can use a condition flag to allow multiple threads
	 * to set interrupts without interfering with each other.
	 * The procedure is: a thread wishing to set an interrupt
	 * acquires the lock.  If the interrupt buffer is not zeroed,
	 * it calls pthread_cond_wait() to release the lock and wait
	 * its turn.  If the buffer is zeroed, it will set the desired
	 * interrupt value and release the lock.
	 * The CPU function will, at the correct point in
	 * its fake cycles, acquire the lock and, if it finds an interrupt
	 * waiting, will act on it and zero the buffer.
	 * In either case, the CPU will call pthread_cond_signal()
	 * prior to releasing the mutex, so that anyone waiting
	 * to write an interrupt to the buffer may do so.
	 * Since each fake CPU cycle ends with a sleep, we don't need to
	 * actively yield control; even if we're on the same core as the
	 * thread that wants to interrupt, it will get a chance to do so.
	 * For Space Invaders, there's only one thread that might signal
	 * an interrupt, but this is extensible.
	 */
	pthread_cond_t* const int_cond;
	pthread_mutex_t* const int_lock;

	uint8_t* const memory; // Points to an array containing the memory.
	uint8_t* const interrupt_buffer; // Points to the buffer where pending
	// interrupts are stored.
	uint8_t* const data_bus;
	uint16_t* const address_bus;

	// Registers!
	uint16_t sp; // Stack pointer
	uint16_t pc; // Program counter;
	uint16_t bc; // The B and C registers.
	uint16_t de; // D and E
	uint16_t hl; // H and L

	uint16_t psw; // A and flags.
	/* PSW is a special register, containing the Accumulator and
	 * the flag byte.  The accumulator is the high byte, and the
	 * flags are the low byte. This doesn't appear to be how the real
	 * hardware does it, but this is how the manual treats it, and it
	 * lets us work the push/pop logic more easily.
	 */
	uint8_t halt_flag; // Flag for the HLT state.
	uint8_t reset_flag;
	/*
	 * The interrupt enable flag has three states, rather than just two:
	 * 0: interrupts disabled
	 * 1: interrupts enabled
	 * 2: interrupts should be enabled after the current opcode completes
	 *
	 * The reason for this is that the EI instruction, which enables
	 * interrupts, enables them following completion of the *following*
	 * opcode.
	 */
	uint8_t interrupt_enable_flag;
};

// The system resources struct is just all the shared pointer members
// of the CPU struct wrapped into a package by themselves, for passing into
// thread initializers.
struct system_resources
{
	pthread_cond_t* interrupt_cond;
	pthread_mutex_t* interrupt_lock;
	uint8_t* memory;	   // Points to an array containing the memory.
	uint8_t* interrupt_buffer; // Points to the buffer where pending
	uint8_t* data_bus;
	uint16_t* address_bus;
};

// Declaration of the CPU thread.

void* cpu_thread_routine(void*);

#endif