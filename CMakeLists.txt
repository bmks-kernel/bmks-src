cmake_minimum_required(VERSION 3.15)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(bmks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
# Added -fno-stack-protector to fix undefined __stack_chk_fail_local
# Added -fno-pie to disable position independent code
set(CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -fno-stack-protector -fno-pie")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot.o
    COMMAND nasm -f elf32 ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm -o ${CMAKE_CURRENT_BINARY_DIR}/boot.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm
)

add_executable(bmks.bin ${CMAKE_CURRENT_BINARY_DIR}/boot.o src/kmain.cpp src/gdt.cpp src/idt.cpp src/pic.cpp src/timer.cpp src/keyboard.cpp src/pmm.cpp)

# Added -no-pie to linker flags
set_target_properties(bmks.bin PROPERTIES LINK_FLAGS "-m32 -nostdlib -no-pie -T ${CMAKE_SOURCE_DIR}/linker.ld")