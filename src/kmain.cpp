#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "paging.h"
#include "heap.h"
#include "pci.h"
#include "ata.h"
#include "mbr.h"
#include "rtc.h"
#include "acpi.h"
#include "syscall.h"

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
    log_info("boot: kernel started");
    
    if (magic != 0x2BADB002) {
        log_info("panic: bad multiboot magic");
        return;
    }
    
    init_gdt();
    log_info("cpu: GDT loaded");
    
    init_idt();
    log_info("cpu: IDT loaded");
    
    pic_remap();
    log_info("pic: 8259A remapped");
    
    init_timer(100); 
    log_info("timer: PIT set to 100Hz");

    rtc_init();
    
    uint32_t total_memory_kb = mb_info->mem_lower + mb_info->mem_upper;
    pmm_init(total_memory_kb);
    log_info("mem: PMM initialized");
    
    init_paging();
    log_info("mem: VMM enabled (8MB identity)");
    
    // Enable interrupts only after critical structures are loaded
    __asm__ __volatile__("sti");
    log_info("cpu: interrupts unmasked");

    // Initialize 16KB Kernel Heap at 0xC0000000
    uint32_t heap_virt = 0xC0000000;
    for (uint32_t i = 0; i < 4; i++) {
        void* phys = pmm_alloc_block();
        vmm_map_page((uint32_t)phys, heap_virt + (i * 4096));
    }
    heap_init(heap_virt, 4 * 4096);
    log_info("mem: heap ready at 0xC0000000");
    
    log_info("pci: scanning bus...");
    pci_scan();

    // Test ATA PIO Driver
    uint8_t sector_buffer[512];
    if (ata_read_sector(0, sector_buffer)) {
        if (sector_buffer[510] == 0x55 && sector_buffer[511] == 0xAA) {
            log_info("ata: successfully read MBR from disk (magic 0xAA55 found)");
        } else {
            log_info("ata: read sector 0 successfully, but disk is unformatted");
        }
    } else {
        log_info("ata: disk not found or timeout");
    }


    acpi_init();
    syscall_init();

    // TEST SYSCALL 1 (Print text)
    const char* sys_msg = "  --> [SYSCALL] Hello from user-space simulation!\n";
    __asm__ __volatile__(
        "int $0x80"
        : 
        : "a" (1), "b" ((uint32_t)sys_msg), "c" (0), "d" (0)
    );

    // TEST SYSCALL 2 (Addition)
    uint32_t sum_result;
    __asm__ __volatile__(
        "int $0x80"
        : "=a" (sum_result)
        : "a" (2), "b" (40), "c" (2), "d" (0)
    );
    
    vga::print("  --> [SYSCALL] Result of 40 + 2 = ");
    vga::print_num(sum_result);
    vga::print("\n");

    mbr_parse();

    
    log_info("boot: init complete. entering idle.");
    
    while (true) {
        __asm__ __volatile__("hlt");
    }
}