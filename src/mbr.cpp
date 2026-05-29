#include "mbr.h"
#include "ata.h"

extern void log_info(const char* msg);

namespace vga {
    extern void print(const char* str);
    extern void print_hex(uint32_t num);
    extern void print_num(uint32_t num);
}

void mbr_parse() {
    uint8_t sector[512];
    
    if (!ata_read_sector(0, sector)) {
        log_info("mbr: failed to read disk");
        return;
    }
    
    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        log_info("mbr: no valid boot signature found");
        return;
    }
    
    log_info("mbr: valid signature 0xAA55 found, parsing partitions...");
    
    // Partition table starts at offset 446
    mbr_partition_t* part = (mbr_partition_t*)&sector[446];
    
    bool has_partitions = false;
    for (int i = 0; i < 4; i++) {
        if (part[i].sector_count > 0) {
            has_partitions = true;
            vga::print("  --> PART "); vga::print_num(i);
            vga::print(" | TYPE: "); vga::print_hex(part[i].type);
            vga::print(" | LBA START: "); vga::print_num(part[i].lba_start);
            vga::print(" | SECTORS: "); vga::print_num(part[i].sector_count);
            vga::print("\n");
        }
    }
    
    if (!has_partitions) {
        log_info("mbr: disk is empty, no partitions defined");
    }
}