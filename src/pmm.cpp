#include <stdint.h>
#include <stddef.h>

extern void log_info(const char* msg);

namespace vga {
    extern void print_num(uint32_t num);
    extern void print(const char* str);
}

#define PAGE_SIZE 4096
#define BLOCKS_PER_BYTE 8
// Max 128MB of RAM supported for now
#define MAX_BLOCKS (128 * 1024 * 1024) / PAGE_SIZE 

uint8_t memory_bitmap[MAX_BLOCKS / BLOCKS_PER_BYTE];
uint32_t max_blocks = 0;
uint32_t used_blocks = 0;

void pmm_init(uint32_t mem_size_kb) {
    max_blocks = (mem_size_kb * 1024) / PAGE_SIZE;
    if (max_blocks > MAX_BLOCKS) {
        max_blocks = MAX_BLOCKS;
    }

    for (uint32_t i = 0; i < MAX_BLOCKS / BLOCKS_PER_BYTE; i++) {
        memory_bitmap[i] = 0;
    }

    
    for (uint32_t i = 0; i < (4 * 1024 * 1024) / PAGE_SIZE; i++) {
        memory_bitmap[i / 8] |= (1 << (i % 8));
        used_blocks++;
    }
    
    log_info("pmm: memory bitmap initialized (4MB reserved)");
}

void* pmm_alloc_block() {
    for (uint32_t i = 0; i < max_blocks; i++) {
        if (!(memory_bitmap[i / 8] & (1 << (i % 8)))) {
            memory_bitmap[i / 8] |= (1 << (i % 8));
            used_blocks++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    log_info("panic: out of memory!");
    return nullptr; 
}

void pmm_free_block(void* ptr) {
    uint32_t addr = (uint32_t)ptr;
    uint32_t block = addr / PAGE_SIZE;
    
    if (memory_bitmap[block / 8] & (1 << (block % 8))) {
        memory_bitmap[block / 8] &= ~(1 << (block % 8));
        used_blocks--;
    }
}
