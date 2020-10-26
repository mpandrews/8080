#include "cpu.h"

extern "C" int hw_in(const uint8_t*, struct cpu_state*);

int foo()
{
	hw_in(nullptr, nullptr);
	return 0;
}
