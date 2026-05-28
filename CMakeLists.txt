cmake_minimum_required(VERSION 3.15)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(bmks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot.o
    COMMAND nasm -f elf32 ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm -o ${CMAKE_CURRENT_BINARY_DIR}/boot.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm
)

add_executable(bmks.bin ${CMAKE_CURRENT_BINARY_DIR}/boot.o src/kmain.cpp)

set_target_properties(bmks.bin PROPERTIES LINK_FLAGS "-m32 -nostdlib -T ${CMAKE_SOURCE_DIR}/linker.ld")