#include <stdint.h>
#include "io.h"

extern void pic_send_eoi(uint8_t irq);

namespace vga {
    extern void print_char(char c);
}

// Basic US QWERTY scancode map (Set 1)
const char scancode_to_char[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

extern "C" void keyboard_handler() {
    // Read from PS/2 data port
    uint8_t scancode = inb(0x60);

    // If highest bit is 0, it's a key press (not a release)
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_to_char)) {
            char c = scancode_to_char[scancode];
            if (c != 0) {
                vga::print_char(c);
            }
        }
    }
    
    // Tell PIC we handled the interrupt
    pic_send_eoi(1);
}