#include <stdint.h>
#include "io.h"
#include "acpi.h"

extern void pic_send_eoi(uint8_t irq);
extern void log_info(const char* msg);

const char scancode_to_char[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

extern "C" void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    // Only process key press (highest bit is 0)
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_to_char)) {
            char c = scancode_to_char[scancode];
            
            // If user presses 's', trigger shutdown
            if (c == 's') {
                log_info("ps2: poweroff hotkey pressed");
                acpi_poweroff();
            }
        }
    }
    
    pic_send_eoi(1);
}