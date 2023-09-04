#pragma once

#include <stdint.h>
#include <stddef.h>

namespace tenessine {
namespace vmm {

void init();
void mapPage(uintptr_t phys, void* virt, uint64_t options);
void mapPage(uintptr_t phys, void* virt, uint64_t options, uintptr_t page_table);

[[maybe_unused]] static inline uintptr_t currentPageTable() {
    uintptr_t ret;
    asm volatile("mov %%cr3, %0" : "=r"(ret));
    return ret;
}

[[maybe_unused]] static void invalidatePage(uintptr_t address) {
    asm volatile("invlpg (%0)" : : "b"(address) : "memory");
}

}
}
