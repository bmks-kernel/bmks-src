#include "rtc.h"
#include "io.h"

extern void log_info(const char* msg);

namespace vga {
    extern void print(const char* str);
    extern void print_num(uint32_t num);
}

time_t global_time;

int get_update_in_progress_flag() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

uint8_t get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void rtc_read_datetime() {
    // Wait until RTC is not busy updating
    while (get_update_in_progress_flag());

    uint8_t second = get_rtc_register(0x00);
    uint8_t minute = get_rtc_register(0x02);
    uint8_t hour   = get_rtc_register(0x04);
    uint8_t day    = get_rtc_register(0x07);
    uint8_t month  = get_rtc_register(0x08);
    uint8_t year   = get_rtc_register(0x09);

    uint8_t registerB = get_rtc_register(0x0B);

    // Convert BCD to binary if necessary
    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
    }

    global_time.second = second;
    global_time.minute = minute;
    global_time.hour = hour;
    global_time.day = day;
    global_time.month = month;
    global_time.year = 2000 + year; // QEMU gives two-digit year
}

void rtc_init() {
    rtc_read_datetime();
    log_info("rtc: hardware CMOS clock synchronized");
    
    vga::print("  --> DATE: ");
    if (global_time.day < 10) vga::print("0");
    vga::print_num(global_time.day); vga::print("/");
    if (global_time.month < 10) vga::print("0");
    vga::print_num(global_time.month); vga::print("/");
    vga::print_num(global_time.year);
    
    vga::print(" TIME: ");
    if (global_time.hour < 10) vga::print("0");
    vga::print_num(global_time.hour); vga::print(":");
    if (global_time.minute < 10) vga::print("0");
    vga::print_num(global_time.minute); vga::print(":");
    if (global_time.second < 10) vga::print("0");
    vga::print_num(global_time.second); vga::print("\n");
}

time_t rtc_get_time() {
    return global_time;
}