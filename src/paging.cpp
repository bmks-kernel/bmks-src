#include "paging.h"
#include <stdint.h>

extern void log_info(const char* msg);

__attribute__((aligned(4096))) uint32_t page_directory[1024];
__attribute__((aligned(4096))) uint32_t first_page_table[1024];

void init_paging() {
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    for (int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 4096) | 3;
    }

    page_directory[0] = ((uint32_t)first_page_table) | 3;

    __asm__ __volatile__(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        : "r"(page_directory)
        : "eax"
    );

    log_info("mmu: paging enabled, first 4MB identity mapped");
}