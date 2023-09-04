#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <mem.hpp>
#include <pmm.hpp>
#include <string.h>
#include <debug.hpp>

namespace tenessine {
namespace pmm {

volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr
};

const char* memmap_entry_type_names[] = {
	"usable",
	"reserved",
	"acpi_reclaimable",
	"acpi_nvs",
	"bad_memory",
	"bootloader_reclaimable",
	"kernel_and_modules",
	"framebuffer"
};

volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = nullptr
};

volatile struct limine_kernel_address_request kernel_address_request = {
	.id = LIMINE_KERNEL_ADDRESS_REQUEST,
	.revision = 0,
	.response = nullptr
};

inline uint64_t* get_bitmap(pmm_descriptors* descriptor) {
    return (uint64_t*)((uintptr_t)(descriptor) + sizeof(pmm_descriptors));
}

inline size_t get_bitmap_size(pmm_descriptors* descriptor) {
    return (descriptor->total_memory_area_pages / 8) + ((descriptor->total_memory_area_pages % 8) ? 1 : 0);
}

pmm_descriptors* first_region;
size_t total_pages;
size_t total_memory_pages;

uintptr_t kernel_base_load_virt = 0xffffffff80000000;
uintptr_t kernel_base_load_phys = 0;

uintptr_t kernel_standard_page_table = 0;

volatile uintptr_t hhdm_slide;

inline uintptr_t convert_phys_to_virt(uintptr_t addr) { return addr + hhdm_slide; }

constexpr bool debugPMM = true;

void init() {
	debug::print("-> PMM INIT\n");
	debug::assert(kernel_address_request.response, "did not get kernel address response");
	debug::assert(kernel_address_request.response->physical_base, "no physical base available");
	debug::assert(kernel_address_request.response->virtual_base, "no virtual base available");
	debug::assert(hhdm_request.response, "did not get hhdm response");
	debug::assert(hhdm_request.response->offset, "did not get hhdm value");

	kernel_base_load_phys = kernel_address_request.response->physical_base;
	kernel_base_load_virt = kernel_address_request.response->virtual_base;
	hhdm_slide = hhdm_request.response->offset;

	debug::printf("--> kernel loaded at %x:%x (hhdm_slide=%x)\n", kernel_base_load_phys, kernel_base_load_virt, hhdm_slide);

	// Iterate through each entry
	int count_usable_regions = 0;
	for(size_t i = 0; i < memmap_request.response->entry_count; i++) {
		auto* entry = memmap_request.response->entries[i];
		if(debugPMM) {
			debug::printf("--> memmap entry %i: type=%s\n---> base:len = %x:%x\n", 
				i, memmap_entry_type_names[entry->type], entry->base, entry->length);
		}

		if(entry->type != LIMINE_MEMMAP_USABLE) { continue; }
		count_usable_regions++;

		// Create the descriptor at the top of the memory space
		if(debugPMM) { debug::printf("---> using this region\n"); }
		auto* descriptor = (pmm_descriptors*)(entry->base + hhdm_slide);
		descriptor->next = nullptr;
		descriptor->hint_offset = 0;
		descriptor->used_pages = 0;

		descriptor->total_memory_area_pages = entry->length / 4096;
		total_memory_pages += descriptor->total_memory_area_pages;

		// First page of useful memory in this region
		descriptor->begin_page = (uintptr_t*)round_to_page_up(entry->base + get_bitmap_size(descriptor) + sizeof(pmm_descriptors) + hhdm_slide);
		// Amount of useful pages in this region
		descriptor->total_pages = (((entry->base + entry->length + hhdm_slide) - (uintptr_t)(descriptor->begin_page)) / 4096);
		total_pages += descriptor->total_pages;

		// Create the bitmap for this region
		uint8_t* bitmap_begin = (uint8_t*)(entry->base + sizeof(pmm_descriptors));
		memset(bitmap_begin, 0, get_bitmap_size(descriptor));
		
		// Add the descriptor to the list
		if(first_region == nullptr) {
			first_region = descriptor;
		} else {
			auto* curr = first_region;
			while(curr->next) { curr = curr->next; }
			curr->next = descriptor;
		}
	}

	debug::printf("--> total amount of memory pages %i, overhead %i\n", total_memory_pages, total_memory_pages - total_pages);
}

uintptr_t allocatePages(int count) {
    // TODO: mutex
    pmm_descriptors* curr = first_region;
    while(curr) {
        auto* bitmap = (uint8_t*)get_bitmap(curr);
        size_t current_block_len = 0;
        for(size_t i = 0; i < curr->total_pages; i++) {
            uint8_t* bitmap_entry = bitmap + (i / 8);
            if(*bitmap_entry & (1 << (i % 8))) {
                current_block_len = 0;
                continue;
            } else { current_block_len++; }

            if(current_block_len >= (size_t)count) {
                // This block is big enough, set them as used
                for(size_t y = 0; y < current_block_len; y++) {
                    int blk = i - y;
                    bitmap[blk / 8] |= (1ULL << (blk % 8));
                }
                uintptr_t addr = ((uintptr_t)(curr->begin_page) + ((i - current_block_len + 1) * 4096)) - hhdm_slide;
                curr->used_pages += count;
                memset((void*)(addr + hhdm_slide), 0, 4096 * count);
                if(debugPMM) { debug::printf("PMM: Allocated %i pages at %x\n", count, addr); }
                return addr;
            }
        }
        // Nothing worked, check next region
        curr = curr->next;
    }
    // TODO: handle OOM
    debug::assert(false, "OOM");
    return 0;
}

void freePages(uintptr_t addr, int count) {
    // Find the region that the pages are in
    pmm_descriptors* curr = first_region;
    while(curr) {
        if((uintptr_t )(curr->begin_page) <= addr && ((uintptr_t)(curr->begin_page) + (curr->total_pages * 4096)) > (addr + (count * 4096))) {
            uint64_t* bitmap = get_bitmap(curr);
            for(int i = 0; i < count; i++) {
                bitmap[(((addr + (i * 4096)) - (uintptr_t )(curr->begin_page)) / 4096) / 64] &= ~(1 << ((((addr + (i * 4096)) - (uintptr_t )(curr->begin_page)) / 4096) % 64));
            }
            return;
        }
        curr = curr->next;
    }
}

}
}
