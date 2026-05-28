#include <stdint.h>
#include <stddef.h>

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

    void print(const char* str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            if (str[i] == '\n') {
                cursor = (cursor / VGA_WIDTH + 1) * VGA_WIDTH;
            } else {
                buffer[cursor++] = (uint16_t)str[i] | (7 << 8);
            }
        }
    }
    
    void print_num(uint32_t num) {
        if (num == 0) {
            print("0");
            return;
        }
        char buf[16];
        int i = 0;
        while (num > 0) {
            buf[i++] = (num % 10) + '0';
            num /= 10;
        }
        while (i > 0) {
            char s[2] = {buf[--i], 0};
            print(s);
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

extern "C" void kmain(uint32_t magic, uint32_t) {
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
    
    __asm__ __volatile__("sti");
    log_info("cpu: hardware interrupts unmasked");
    
    log_info("bmks: entering idle state");
    
    uint32_t last_tick = 0;
    while (true) {
        __asm__ __volatile__("hlt");
        
        if (timer_ticks >= last_tick + 100) {
            log_info("bmks: 1 second uptime");
            last_tick = timer_ticks;
        }
    }
}