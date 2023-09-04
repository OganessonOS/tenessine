#include <limine.h>
#include <stdint.h>
#include <stddef.h>
#include <mem.hpp>
#include <pmm.hpp>
#include <vmm.hpp>
#include <string.h>
#include <debug.hpp>

namespace tenessine {
namespace vmm {

void init() {
	debug::print("-> VMM INIT\n");

	// Allocate our initial page table
	uintptr_t page_table_phys = pmm::allocatePages(1);
	auto* page_table = (uint64_t*)(page_table_phys + pmm::hhdm_slide);
	memset(page_table, 0, 0x1000);

	debug::printf("--> initial page table at %x\n", page_table_phys);

	// Preallocate all level 4 entries from 256-512
	// This makes sharing page tables easier, as we can guarantee every possible kernel address will share a level 3 table
	for(size_t i = 256; i < 512; i++) {
		uintptr_t table = pmm::allocatePages(1);
		memset((void*)(table + pmm::hhdm_slide), 0, 0x1000);
		page_table[i] = table | 0b11;
	}

	debug::printf("---> mapping memmap entries\n");

	// Iterate through the limine memmap and map everything we need from there
	debug::assert(pmm::memmap_request.response, "memmap response not available in vmm?");
	for(size_t i = 0; i < pmm::memmap_request.response->entry_count; i++) {
		auto* entry = pmm::memmap_request.response->entries[i];
		debug::printf("----> mapping HHDM for %x:%x\n", entry->base, entry->length);
		// Map HHDM
		for(uintptr_t curr = entry->base; curr < (entry->base + entry->length); curr += 0x1000) {
			if(entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
				debug::printf("-----> %x -> %x (hhdm: %x)\n", curr, curr + pmm::hhdm_slide, pmm::hhdm_slide);
			mapPage(curr, (void*)(curr + pmm::hhdm_slide), page_table_phys);
		}

		// Map the kernel as well
		if(entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
			debug::printf("----> mapping kernel area\n");
			for(uintptr_t curr = entry->base; curr < (entry->base + entry->length); curr += 0x1000) {
				mapPage(curr, (void*)(curr + pmm::kernel_base_load_virt - pmm::kernel_base_load_phys), page_table_phys);
			}
		}

		// Identity map bootloader reclaimable memory and the framebuffer
		if(entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
			for(uintptr_t curr = entry->base; curr < (entry->base + entry->length); curr += 0x1000) {
				mapPage(curr, (void*)curr, page_table_phys);
			}
		}
	}
}

void mapPage(uintptr_t phys, void* virt, uint64_t options) {
	mapPage(phys, virt, options, currentPageTable());
}

void mapPage(uintptr_t phys, void* virt, uint64_t options, uintptr_t page_table) {
    uint64_t lvl4 = ((uintptr_t)virt >> 39ULL) & 0b111111111;
    uint64_t lvl3 = ((uintptr_t)virt >> 30ULL) & 0b111111111;
    uint64_t lvl2 = ((uintptr_t)virt >> 21ULL) & 0b111111111;
    uint64_t lvl1 = ((uintptr_t)virt >> 12ULL) & 0b111111111;

    uint64_t* lvl4_table;
    if(page_table & (1ULL << 63)) { lvl4_table = (uint64_t*)page_table; } else { lvl4_table = (uint64_t*)(page_table + pmm::hhdm_slide); }
    if(~(lvl4_table[lvl4]) & 1) {
        uintptr_t table = pmm::allocatePages(1);
        memset((void*)(table + pmm::hhdm_slide), 0, 4096);
        lvl4_table[lvl4] = table | 0b111;
        invalidatePage(table + pmm::hhdm_slide);
    }
    uint64_t* lvl3_table = (uint64_t*)((lvl4_table[lvl4] & 0xffffffffff000) + pmm::hhdm_slide);
    if(~(lvl3_table[lvl3]) & 1) {
        uintptr_t table = pmm::allocatePages(1);
        memset((void*)(table + pmm::hhdm_slide), 0, 4096);
        lvl3_table[lvl3] = table | 0b111;
        invalidatePage(table + pmm::hhdm_slide);
    }
    uint64_t* lvl2_table = (uint64_t*)((lvl3_table[lvl3] & 0xffffffffff000) + pmm::hhdm_slide);
    if(~(lvl2_table[lvl2]) & 1) {
        uintptr_t table = pmm::allocatePages(1);
        memset((void*)(table + pmm::hhdm_slide), 0, 4096);
        lvl2_table[lvl2] = table | 0b111;
        invalidatePage(table + pmm::hhdm_slide);
    }
    uint64_t* lvl1_table = (uint64_t*)((lvl2_table[lvl2] & 0xffffffffff000) + pmm::hhdm_slide);
    if(~options & 1) { lvl1_table[lvl1] = 0; return; }
    // TODO: assert
    lvl1_table[lvl1] = (phys & 0xFFFFFFFFFF000) | options;
    invalidatePage(((uintptr_t)virt & 0xFFFFFFFFFF000));
}

}
}
