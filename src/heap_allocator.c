#include "heap_allocator.h"
#include "common.h"

#define HEAP_START 0x80000000   // Adjust based on linker script
#define HEAP_SIZE  0x01000000   // 16 MB heap for example

heap_block_t *free_list;

void heap_init() {
    free_list = (heap_block_t *)HEAP_START;
    free_list->size = HEAP_SIZE - sizeof(heap_block_t);
    free_list->free = 1;
    free_list->next = NULL;
}

void *malloc(size_t size) {
    size = (size + 7) & ~7; // Align to 8 bytes

    heap_block_t *curr = free_list;
    heap_block_t *prev = NULL;

    while (curr) {
        if (curr->free && curr->size >= size) {
            // Split block if large enough
            if (curr->size > size + sizeof(heap_block_t)) {
                heap_block_t *new_block = (heap_block_t *)((char *)curr + sizeof(heap_block_t) + size);
                new_block->size = curr->size - size - sizeof(heap_block_t);
                new_block->free = 1;
                new_block->next = curr->next;
                curr->next = new_block;
                curr->size = size;
            }
            curr->free = 0;
            return (char *)curr + sizeof(heap_block_t);
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL; // Out of memory
}

void free(void *ptr) {
    if (!ptr) return;

    heap_block_t *block = (heap_block_t *)((char *)ptr - sizeof(heap_block_t));
    block->free = 1;

    // Merge adjacent free blocks
    heap_block_t *curr = free_list;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += sizeof(heap_block_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}