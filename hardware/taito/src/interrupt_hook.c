#include "cpu.h"
#include "taito_struct.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_MEMORY_OFFSET	 0x2400
#define VIDEO_MEMORY_TOP_SIZE	 0xC00
#define VIDEO_MEMORY_BOTTOM_SIZE 0x1000

#define LINE_96_INTERRUPT  0xcf
#define LINE_224_INTERRUPT 0xd7

int hw_interrupt_hook(const uint8_t* opcode,
		struct cpu_state* cpu,
		int (*op_func)(const uint8_t*, struct cpu_state*))

{
	struct taito_struct* tstruct = (struct taito_struct*) cpu->hw_struct;

	/* If we've caught 0xcf, RST1, then it's time to stop rendering the
	 * top half of the screen.  Which also means we should update
	 * the video buffer with the top half's contents.
	 */
	if (opcode[0] == LINE_96_INTERRUPT)
	{
		pthread_mutex_lock(tstruct->vbuffer_lock);
		memcpy(tstruct->vbuffer,
				cpu->memory + VIDEO_MEMORY_OFFSET,
				VIDEO_MEMORY_TOP_SIZE);
		pthread_mutex_unlock(tstruct->vbuffer_lock);
		pthread_cond_signal(tstruct->vbuffer_cond);
	}
	/* And the same basic idea if we catch 0xd7, which is the bottom half
	 * interrupt.  We copy that memory into the vbuffer.
	 */
	else if (opcode[0] == LINE_224_INTERRUPT)
	{

		pthread_mutex_lock(tstruct->vbuffer_lock);
		memcpy(tstruct->vbuffer + VIDEO_MEMORY_TOP_SIZE,
				cpu->memory + VIDEO_MEMORY_OFFSET
						+ VIDEO_MEMORY_TOP_SIZE,
				VIDEO_MEMORY_BOTTOM_SIZE);
		pthread_mutex_unlock(tstruct->vbuffer_lock);
		pthread_cond_signal(tstruct->vbuffer_cond);
	}

	// Whether we updated the video buffer or not, we still execute the
	// interrupt.
	return op_func(opcode, cpu);
}
