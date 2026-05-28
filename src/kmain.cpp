#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "paging.h"

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

extern "C" void kmain(uint32_t magic, multiboot_info* mb_info) {
    vga::clear();
    log_info("bmks: early boot initialized");
    
    if (magic != 0x2BADB002) {
        log_info("panic: invalid multiboot magic number");
        return;
    }
    log_info("multiboot: magic ok");
    
    init_gdt();
    log_info("cpu: GDT loaded");
    
    init_idt();
    pic_remap();
    init_timer(100); 
    
    uint32_t total_memory_kb = mb_info->mem_lower + mb_info->mem_upper;
    
    vga::print("["); vga::print_num(timer_ticks); vga::print("] ");
    vga::print("ram: detected "); vga::print_num(total_memory_kb / 1024); vga::print(" MB\n");
    
    pmm_init(total_memory_kb);
    
    void* ptr = pmm_alloc_block();
    if (ptr) {
        log_info("pmm: successfully allocated 4KB physical block");
    }

    init_paging();
    
    __asm__ __volatile__("sti");
    log_info("cpu: hardware interrupts unmasked");
    log_info("ps2: keyboard initialized");
    log_info("bmks: entering idle state");
    
    while (true) {
        __asm__ __volatile__("hlt");
    }
}