#pragma once

constexpr uint64_t round_to_page_up(uint64_t addr) {
    return ((addr + 4096 - 1) & ~(4096 - 1));
}
