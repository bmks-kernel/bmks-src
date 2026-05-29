#include "heap.h"

struct heap_segment_t {
    size_t size;
    bool is_free;
    heap_segment_t* next;
    heap_segment_t* prev;
};

heap_segment_t* heap_start = nullptr;

void heap_init(uint32_t start_addr, uint32_t size) {
    heap_start = (heap_segment_t*)start_addr;
    heap_start->size = size - sizeof(heap_segment_t);
    heap_start->is_free = true;
    heap_start->next = nullptr;
    heap_start->prev = nullptr;
}

void* kmalloc(size_t size) {
    if (size == 0) return nullptr;

    heap_segment_t* current = heap_start;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            if (current->size > size + sizeof(heap_segment_t) + 1) {
                heap_segment_t* new_seg = (heap_segment_t*)((uint8_t*)current + sizeof(heap_segment_t) + size);
                new_seg->size = current->size - size - sizeof(heap_segment_t);
                new_seg->is_free = true;
                new_seg->next = current->next;
                new_seg->prev = current;
                
                if (current->next) {
                    current->next->prev = new_seg;
                }
                current->next = new_seg;
                current->size = size;
            }
            current->is_free = false;
            return (void*)((uint8_t*)current + sizeof(heap_segment_t));
        }
        current = current->next;
    }
    
    return nullptr;
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    heap_segment_t* segment = (heap_segment_t*)((uint8_t*)ptr - sizeof(heap_segment_t));
    segment->is_free = true;

    if (segment->next && segment->next->is_free) {
        segment->size += segment->next->size + sizeof(heap_segment_t);
        segment->next = segment->next->next;
        if (segment->next) {
            segment->next->prev = segment;
        }
    }
    
    if (segment->prev && segment->prev->is_free) {
        segment->prev->size += segment->size + sizeof(heap_segment_t);
        segment->prev->next = segment->next;
        if (segment->next) {
            segment->next->prev = segment->prev;
        }
    }
}