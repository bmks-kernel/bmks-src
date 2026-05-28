#include <stdint.h>
#include "io.h"

extern void pic_send_eoi(uint8_t irq);

volatile uint32_t timer_ticks = 0;

extern "C" void timer_handler() {
    timer_ticks++;
    pic_send_eoi(0);
}

void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}