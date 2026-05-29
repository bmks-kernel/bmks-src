#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "paging.h"
#include "heap.h"
#include "pci.h"

extern volatile uint32_t timer_ticks;

namespace vga {
    volatile uint16_t* buffer = (uint16_t*)0xB8000;
    size_t cursor = 0;
    const size_t VGA_WIDTH = 80;
    const size_t VGA_HEIGHT = 25;

    void clear() {
        for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            buffer[i] = (uint16_t)' ' | (7 << 8); 
        }
        cursor = 0;
    }

    void print_char(char c) {
        if (c == '\n') {
            cursor = (cursor / VGA_WIDTH + 1) * VGA_WIDTH;
        } else if (c == '\b') {
            if (cursor > 0) {
                cursor--;
                buffer[cursor] = (uint16_t)' ' | (7 << 8);
            }
        } else {
            buffer[cursor++] = (uint16_t)c | (7 << 8);
        }
    }

    void print(const char* str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            print_char(str[i]);
        }
    }
    
    void print_num(uint32_t num) {
        if (num == 0) {
            print_char('0');
            return;
        }
        char buf[16];
        int i = 0;
        while (num > 0) {
            buf[i++] = (num % 10) + '0';
            num /= 10;
        }
        while (i > 0) {
            print_char(buf[--i]);
        }
    }

    void print_hex(uint32_t num) {
        print("0x");
        if (num == 0) { print_char('0'); return; }
        const char* hex_chars = "0123456789ABCDEF";
        char buf[8];
        int i = 0;
        while (num > 0 && i < 8) {
            buf[i++] = hex_chars[num & 0x0F];
            num >>= 4;
        }
        while (i > 0) {
            print_char(buf[--i]);
        }
    }
}

void log_info(const char* msg) {
    vga::print("[");
    vga::print_num(timer_ticks);
    vga::print("] ");
    vga::print(msg);
    vga::print("\n");
}

void init_gdt();
void init_idt();
void pic_remap();
void init_timer(uint32_t freq);
void pmm_init(uint32_t mem_size_kb);
void* pmm_alloc_block();
void vmm_map_page(uint32_t phys, uint32_t virt);

extern "C" void kmain(uint32_t magic, multiboot_info* mb_info) {
    vga::clear();
    
    if (magic != 0x2BADB002) return;
    
    init_gdt();
    init_idt();
    pic_remap();
    init_timer(100); 
    
    uint32_t total_memory_kb = mb_info->mem_lower + mb_info->mem_upper;
    pmm_init(total_memory_kb);
    init_paging();
    
    __asm__ __volatile__("sti");

    uint32_t heap_virt = 0xC0000000;
    for (uint32_t i = 0; i < 4; i++) {
        void* phys = pmm_alloc_block();
        vmm_map_page((uint32_t)phys, heap_virt + (i * 4096));
    }
    heap_init(heap_virt, 4 * 4096);
    
    log_info("bmks: kernel foundation loaded");

    // SCAN HARDWARE
    pci_scan();

    while (true) {
        __asm__ __volatile__("hlt");
    }
}