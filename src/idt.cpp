#include <stdint.h>

extern void log_info(const char* msg);
extern "C" void default_isr_stub();

struct idt_entry {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

__attribute__((aligned(0x10))) idt_entry idt[256];
idtr idtr_reg;

extern "C" void isr_handler() {
    log_info("cpu: interrupt triggered");
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    uint32_t isr_addr = (uint32_t)isr;
    idt[vector].isr_low = isr_addr & 0xFFFF;
    idt[vector].kernel_cs = 0x08;
    idt[vector].reserved = 0;
    idt[vector].attributes = flags;
    idt[vector].isr_high = (isr_addr >> 16) & 0xFFFF;
}

void init_idt() {
    idtr_reg.base = (uint32_t)&idt[0];
    idtr_reg.limit = (uint16_t)sizeof(idt_entry) * 256 - 1;

    for (uint16_t vector = 0; vector < 256; vector++) {
        idt_set_descriptor(vector, (void*)default_isr_stub, 0x8E);
    }

    __asm__ __volatile__("lidt %0" : : "m"(idtr_reg));
    __asm__ __volatile__("sti");
}