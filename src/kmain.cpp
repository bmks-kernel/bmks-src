#include <stdint.h>
#include <stddef.h>

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
}

void log_info(const char* msg) {
    vga::print("[    0.000000] ");
    vga::print(msg);
    vga::print("\n");
}

extern "C" void kmain(uint32_t magic, uint32_t) {
    vga::clear();
    
    log_info("bmks: early boot initialized");
    
    if (magic != 0x2BADB002) {
        log_info("panic: invalid multiboot magic number");
        return;
    }
    
    log_info("multiboot: magic ok");
    log_info("vga: text buffer mapped to 0xB8000");
    log_info("cpu: entered 32-bit protected mode");
    log_info("cpu: halting in idle loop");
    
    while (true) {
        __asm__ __volatile__("hlt");
    }
}