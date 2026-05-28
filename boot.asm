MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16KB stack
stack_top:

section .text
bits 32 ; enforce 32-bit mode
global _start
extern kmain

_start:
    ; setup stack pointer
    mov esp, stack_top
    
    ; pass multiboot magic and info structure to c++
    push ebx
    push eax
    
    call kmain

    ; halt cpu if kmain returns
    cli
.hang:
    hlt
    jmp .hang



    global default_isr_stub
extern isr_handler

default_isr_stub:
    pushad
    cld
    call isr_handler
    popad
    iretd



global irq0_stub
extern timer_handler

irq0_stub:
    pushad
    cld
    call timer_handler
    popad
    iretd


global irq1_stub
extern keyboard_handler

irq1_stub:
    pushad
    cld
    call keyboard_handler
    popad
    iretd



global page_fault_stub
extern page_fault_handler

page_fault_stub:
    pushad
    cld
    call page_fault_handler
    popad
    add esp, 4
    iretd