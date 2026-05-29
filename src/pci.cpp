#include "pci.h"
#include "io.h"

extern void log_info(const char* msg);

namespace vga {
    extern void print(const char* str);
    extern void print_hex(uint32_t num);
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_scan() {
    log_info("pci: scanning bus for devices...");
    
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t vendor_device = pci_read_config(bus, slot, 0, 0);
            uint16_t vendor = vendor_device & 0xFFFF;
            
            if (vendor != 0xFFFF) {
                uint16_t device = vendor_device >> 16;
                
                vga::print("  --> PCI DEV [BUS ");
                vga::print_hex(bus);
                vga::print(" SLOT ");
                vga::print_hex(slot);
                vga::print("] VENDOR: ");
                vga::print_hex(vendor);
                vga::print(" DEV_ID: ");
                vga::print_hex(device);
                vga::print("\n");
            }
        }
    }
    
    log_info("pci: scan complete");
}