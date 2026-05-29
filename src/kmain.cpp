#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "paging.h"
#include "task.h"

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

void delay(uint32_t ticks) {
    uint32_t start = timer_ticks;
    while (timer_ticks < start + ticks) {
        __asm__ __volatile__("hlt");
    }
}

void init_gdt();
void init_idt();
void pic_remap();
void init_timer(uint32_t freq);
void pmm_init(uint32_t mem_size_kb);

extern task_t tasks[2];

void task1() {
    while (true) {
        log_info("Task 1 is running");
        delay(50); 
        task_yield();
    }
}

void task2() {
    while (true) {
        log_info("Task 2 is running");
        delay(50); 
        task_yield();
    }
}

extern "C" void kmain(uint32_t magic, multiboot_info* mb_info) {
    vga::clear();
    log_info("bmks: early boot initialized");
    
    if (magic != 0x2BADB002) return;
    
    init_gdt();
    init_idt();
    pic_remap();
    init_timer(100); 
    
    uint32_t total_memory_kb = mb_info->mem_lower + mb_info->mem_upper;
    pmm_init(total_memory_kb);
    init_paging();
    
    __asm__ __volatile__("sti");
    
    log_info("scheduler: initializing tasks");
    task_init(&tasks[0], task1);
    task_init(&tasks[1], task2);
    
    log_info("scheduler: starting multi-threading");
    start_tasks();
    
    while (true) {
        __asm__ __volatile__("hlt");
    }
}