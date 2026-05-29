section .text
global syscall_stub
extern syscall_handler

syscall_stub:
    pushad
    
    ; Pass standard syscall arguments (arg3, arg2, arg1, syscall_num)
    push edx
    push ecx
    push ebx
    push eax
    
    call syscall_handler
    
    add esp, 16
    
    ; Overwrite the EAX value saved by pushad so the caller gets the return value
    mov [esp + 28], eax 
    
    popad
    iretd