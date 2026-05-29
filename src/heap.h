#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init(uint32_t start_addr, uint32_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);