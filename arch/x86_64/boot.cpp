#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include <pmm.hpp>
#include <vmm.hpp>
#include <debug.hpp>

typedef void (*ctor_constructor)();
extern "C" ctor_constructor start_ctors;
extern "C" ctor_constructor end_ctors;

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = nullptr
};


static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

extern "C" void _start(void) {
	tenessine::debug::print("Tenessine kernel START\n");
	tenessine::pmm::init();
	tenessine::vmm::init();
    hcf();
}
