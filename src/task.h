#pragma once
#include <stdint.h>

struct task_t {
    uint32_t* esp;
    uint32_t stack[1024]; // 4KB stack per task
};

void task_init(task_t* t, void (*entry)());
void task_yield();
void start_tasks();