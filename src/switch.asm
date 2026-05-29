section .text
global context_switch

; void context_switch(uint32_t** old_esp, uint32_t* new_esp);
context_switch:
    push ebp
    push ebx
    push esi
    push edi

    ; Save current ESP to old_esp pointer
    mov eax, [esp + 20]
    mov [eax], esp

    ; Load new ESP
    mov esp, [esp + 24]

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret