#include <stdint.h>

extern uint32_t page_directory[1024];
extern void* pmm_alloc_block();

void vmm_map_page(uint32_t phys, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (!(page_directory[pd_index] & 1)) {
        uint32_t* new_table = (uint32_t*)pmm_alloc_block();
        if (!new_table) return;
        for (int i = 0; i < 1024; i++) {
            new_table[i] = 0;
        }
        page_directory[pd_index] = ((uint32_t)new_table) | 3;
    }

    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
    page_table[pt_index] = (phys & ~0xFFF) | 3;

    __asm__ __volatile__("invlpg (%0)" : : "r"(virt) : "memory");
}