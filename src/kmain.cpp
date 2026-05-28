void init_gdt();
void init_idt();

extern "C" void kmain(uint32_t magic, uint32_t) {
    vga::clear();
    log_info("bmks: early boot initialized");
    
    if (magic != 0x2BADB002) {
        log_info("panic: invalid multiboot magic number");
        return;
    }
    log_info("multiboot: magic ok");
    
    init_gdt();
    log_info("cpu: GDT loaded, flat memory model active");
    
    init_idt();
    log_info("cpu: IDT loaded, interrupts enabled");
    
    __asm__ __volatile__("int $0x03");
    
    log_info("cpu: halting in idle loop");
    
    while (true) {
        __asm__ __volatile__("hlt");
    }
}