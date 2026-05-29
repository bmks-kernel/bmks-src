#include "task.h"

extern "C" void context_switch(uint32_t** old_esp, uint32_t* new_esp);

task_t tasks[2];
int current_task = 0;
uint32_t* scheduler_esp;

void task_init(task_t* t, void (*entry)()) {
    uint32_t* sp = &t->stack[1024];
    
    *(--sp) = (uint32_t)entry; // Return EIP
    *(--sp) = 0;               // EBP
    *(--sp) = 0;               // EBX
    *(--sp) = 0;               // ESI
    *(--sp) = 0;               // EDI
    
    t->esp = sp;
}

void task_yield() {
    int old = current_task;
    current_task = (current_task == 0) ? 1 : 0;
    
    context_switch(&tasks[old].esp, tasks[current_task].esp);
}

void start_tasks() {
    current_task = 0;
    context_switch(&scheduler_esp, tasks[0].esp);
}