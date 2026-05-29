section .text

global context_switch
global tss_flush
global jump_to_usermode

; ---------------------------------------------------------
; Context Switch for Multitasking
; ---------------------------------------------------------
context_switch:
    push ebp
    push ebx
    push esi
    push edi

    mov eax, [esp + 20]
    mov [eax], esp
    mov esp, [esp + 24]

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; ---------------------------------------------------------
; Load Task State Segment (TSS)
; ---------------------------------------------------------
tss_flush:
    mov ax, 0x2B  ; TSS Segment (5th GDT entry) with RPL 3
    ltr ax
    ret

; ---------------------------------------------------------
; Drop privileges and jump to Ring 3
; ---------------------------------------------------------
jump_to_usermode:
    cli
    mov ax, 0x23    ; User Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; FIX: Save the original stack pointer before we modify it!
    ; This ESP points exactly to the return address back to kmain.
    mov eax, esp
    
    ; Build stack frame for iretd
    push 0x23       ; User SS
    push eax        ; User ESP (Corrected!)
    pushfd          ; Save EFLAGS
    
    ; Enable interrupts in EFLAGS (IF bit = 0x200)
    pop ebx
    or ebx, 0x200
    push ebx
    
    push 0x1B       ; User Code Segment
    push .user_mode ; EIP to jump to
    iretd           ; Drop privileges
    
.user_mode:
    ret             ; Pops the correct return address and jumps to kmain