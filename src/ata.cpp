#include "ata.h"
#include "io.h"

bool ata_wait_bsy() {
    uint32_t timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(0x1F7);
        if (status == 0xFF) return false; // Floating bus (no device attached)
        if (!(status & 0x80)) return true; // BSY bit is clear
    }
    return false; // Timeout
}

bool ata_wait_drq() {
    uint32_t timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(0x1F7);
        if (status & 0x08) return true; // DRQ bit set (ready for data)
        if (status & 0x01) return false; // ERR bit set
    }
    return false; // Timeout
}

bool ata_read_sector(uint32_t lba, uint8_t* buffer) {
    outb(0x3F6, 0x02); // Disable interrupts
    
    if (!ata_wait_bsy()) return false;
    
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    
    outb(0x1F7, 0x20); // READ SECTORS command

    if (!ata_wait_bsy()) return false;
    if (!ata_wait_drq()) return false;

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }
    return true;
}