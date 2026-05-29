#include "syscall.h"
#include <stdint.h>

extern void log_info(const char* msg);

namespace vga {
    extern void print(const char* str);
    extern void print_num(uint32_t num);
}

#define MAX_SYSCALLS 3

extern "C" uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    if (syscall_num >= MAX_SYSCALLS) {
        log_info("syscall: invalid system call number requested");
        return (uint32_t)-1;
    }

    switch (syscall_num) {
        case 0: // sys_ping
            log_info("syscall: sys_ping executed");
            return 0;
            
        case 1: // sys_print (arg1 = char* pointer)
            vga::print((const char*)arg1);
            return 0;
            
        case 2: // sys_add (arg1 = a, arg2 = b)
            return arg1 + arg2;
            
        default:
            return 0;
    }
}

void syscall_init() {
    log_info("kernel: syscall interface ready (int 0x80)");
}