#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

namespace tenessine {
namespace pmm {

struct pmm_descriptors {
    pmm_descriptors* next;
    uintptr_t* begin_page;
    size_t hint_offset;
    size_t used_pages;
    size_t total_pages;
    size_t total_memory_area_pages;
} __attribute__((packed));

extern pmm_descriptors* first_region;
extern volatile uintptr_t hhdm_slide;
extern uintptr_t kernel_base_load_virt;
extern uintptr_t kernel_base_load_phys;
extern volatile struct limine_memmap_request memmap_request;

extern const char* memmap_entry_type_names[];

void init();
uintptr_t allocatePages(int count);
void freePages(uintptr_t addr, int count);

}
}
