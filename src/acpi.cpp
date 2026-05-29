#include "acpi.h"
#include "io.h"
#include <stdint.h>

extern void log_info(const char* msg);

extern void vmm_map_page(uint32_t phys, uint32_t virt);

bool memcmp(const void* ptr1, const char* str, int len) {
    const char* p1 = (const char*)ptr1;
    for (int i = 0; i < len; i++) {
        if (p1[i] != str[i]) return false;
    }
    return true;
}

uint32_t pm1a_cnt_port = 0;

void acpi_init() {
    uint8_t* rsdp = nullptr;
    
    for (uint8_t* ptr = (uint8_t*)0x000E0000; ptr < (uint8_t*)0x00100000; ptr += 16) {
        if (memcmp(ptr, "RSD PTR ", 8)) {
            rsdp = ptr;
            break;
        }
    }

    if (!rsdp) {
        log_info("acpi: RSDP not found");
        return;
    }

    log_info("acpi: RSDP descriptor found");

    uint32_t rsdt_addr = *(uint32_t*)(rsdp + 16);
    
    // FIX: Map the physical address of RSDT into virtual memory before reading
    // We map 2 pages (8KB) just in case the table crosses a page boundary
    uint32_t rsdt_page = rsdt_addr & ~0xFFF;
    vmm_map_page(rsdt_page, rsdt_page);
    vmm_map_page(rsdt_page + 4096, rsdt_page + 4096);

    uint8_t* rsdt = (uint8_t*)rsdt_addr;

    if (!memcmp(rsdt, "RSDT", 4)) {
        log_info("acpi: invalid RSDT signature");
        return;
    }

    uint32_t entries = (*(uint32_t*)(rsdt + 4) - 36) / 4;
    uint32_t* ptrs = (uint32_t*)(rsdt + 36);

    for (uint32_t i = 0; i < entries; i++) {
        uint32_t table_addr = ptrs[i];
        
        // FIX: Map each specific ACPI table before trying to read it
        uint32_t table_page = table_addr & ~0xFFF;
        vmm_map_page(table_page, table_page);
        vmm_map_page(table_page + 4096, table_page + 4096);

        uint8_t* table = (uint8_t*)table_addr;
        if (memcmp(table, "FACP", 4)) {
            log_info("acpi: FACP table located");
            
            uint32_t smi_cmd = *(uint32_t*)(table + 46);
            uint8_t acpi_enable = *(table + 52);
            pm1a_cnt_port = *(uint32_t*)(table + 64);

            if (smi_cmd != 0 && acpi_enable != 0) {
                outb(smi_cmd, acpi_enable);
            }
            
            log_info("acpi: power management ready");
            return;
        }
    }
    
    log_info("acpi: FACP missing, shutdown unavailable");
}

void acpi_poweroff() {
    if (pm1a_cnt_port == 0) {
        log_info("acpi: cannot shutdown, PM1a_CNT is 0");
        return;
    }
    log_info("acpi: sending ACPI shutdown signal...");
    
    outw(pm1a_cnt_port, 0x2000);
}