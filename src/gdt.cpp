#include <stdint.h>

struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


struct tss_entry_t {
    uint32_t prev_tss;
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap, iomap_base;
} __attribute__((packed));

gdt_entry_t gdt[6];
gdt_ptr_t gdt_ptr;
tss_entry_t tss_entry;

extern "C" void tss_flush();

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code (0x08)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data (0x10)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code   (0x18)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data   (0x20)

    
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry);
    gdt_set_gate(5, base, limit, 0xE9, 0x00);

    uint8_t* tss_ptr = (uint8_t*)&tss_entry;
    for (uint32_t i = 0; i < sizeof(tss_entry); i++) tss_ptr[i] = 0;

    tss_entry.ss0  = 0x10; 
    tss_entry.esp0 = 0;    
    tss_entry.iomap_base = sizeof(tss_entry);

    __asm__ __volatile__(
        "lgdt %0\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $flush_gdt\n"
        "flush_gdt:\n"
        : : "m"(gdt_ptr)
    );

    tss_flush();
}


void tss_set_stack(uint32_t kstack) {
    tss_entry.esp0 = kstack;
}