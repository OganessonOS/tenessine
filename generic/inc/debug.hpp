#pragma once

namespace tenessine {
namespace debug {

void print(const char* msg);
void printf(const char* format, ...);

static inline void assert(bool cond, const char* msg) {
	if(!cond) {
		printf("ASSERT FAILED: %s\nKernel halted.\n", msg);
		asm volatile("cli");
		for(;;) { asm volatile("pause; hlt;"); }
	}
}

}
}
