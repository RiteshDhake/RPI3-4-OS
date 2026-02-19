#include "heap_allocator.h"
#include "common.h"
#include"mem.h"
#include"mm.h"
#include"printf.h"


// #define HEAP_START 0x80000000   // Adjust based on linker script
// #define HEAP_SIZE  0x01000000   // 16 MB heap for example

heap_block_t *free_list;

void heap_init() {
    free_list = NULL;
}

static heap_block_t* heap_expand() {
    void* page = allocate_memory(PAGE_SIZE);
    if (!page)
        return NULL;

    heap_block_t* block = (heap_block_t*)page;
    block->size = PAGE_SIZE - sizeof(heap_block_t);
    block->free = 1;
    block->next = NULL;

    return block;
}

void *malloc(size_t size) {
    if (size == 0)
        return NULL;

    size = (size + 7) & ~7; // 8-byte alignment

    heap_block_t *curr = free_list;
    heap_block_t *prev = NULL;

    // Search free list
    while (curr) {
        if (curr->free && curr->size >= size)
            break;
        prev = curr;
        curr = curr->next;
    }

    // If no suitable block found → expand heap
    if (!curr) {
        heap_block_t* new_block = heap_expand();
        if (!new_block)
            return NULL;

        if (!free_list)
            free_list = new_block;
        else
            prev->next = new_block;

        curr = new_block;
    }

    // Split if large enough
    if (curr->size > size + sizeof(heap_block_t)) {
        heap_block_t *new_block =
            (heap_block_t *)((char *)curr + sizeof(heap_block_t) + size);

        new_block->size = curr->size - size - sizeof(heap_block_t);
        new_block->free = 1;
        new_block->next = curr->next;

        curr->next = new_block;
        curr->size = size;
    }

    curr->free = 0;

    return (char *)curr + sizeof(heap_block_t);
}

void free(void *ptr) {
    if (!ptr)
        return;

    heap_block_t *block =
        (heap_block_t *)((char *)ptr - sizeof(heap_block_t));

    block->free = 1;

    // Merge physically adjacent free blocks
    heap_block_t *curr = free_list;

    while (curr && curr->next) {
        if (curr->free && curr->next->free &&
            (char*)curr + sizeof(heap_block_t) + curr->size ==
            (char*)curr->next) {

            curr->size += sizeof(heap_block_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}




void heap_stress_test() {
    printf("Heap allocator test start\n");

    void* ptrs[HEAP_TEST_COUNT];

    // Allocate small blocks
    for (int i = 0; i < HEAP_TEST_COUNT; i++) {
        ptrs[i] = malloc(64);
        if (!ptrs[i]) {
            printf("Heap OOM at %d\n", i);
            break;
        }

        memzero((unsigned long)ptrs[i], 64);

        // Fill pattern
        unsigned char* p = (unsigned char*)ptrs[i];
        for (int j = 0; j < 64; j++)
            p[j] = 0x55;
    }

    // Verify pattern
    for (int i = 0; i < HEAP_TEST_COUNT; i++) {
        if (!ptrs[i])
            break;

        unsigned char* p = (unsigned char*)ptrs[i];
        for (int j = 0; j < 64; j++) {
            if (p[j] != 0x55) {
                printf("Heap corruption at block %d offset %d\n", i, j);
                while (1);
            }
        }
    }

    // Free every other block (fragmentation test)
    for (int i = 0; i < HEAP_TEST_COUNT; i += 2) {
        if (ptrs[i])
            free(ptrs[i]);
    }

    // Allocate again to reuse holes
    for (int i = 0; i < HEAP_TEST_COUNT / 2; i++) {
        void* p = malloc(32);
        if (!p) {
            printf("Heap reuse failed\n");
            while (1);
        }

        memzero((unsigned long)p, 32);
    }

    // Free remaining
    for (int i = 1; i < HEAP_TEST_COUNT; i += 2) {
        if (ptrs[i])
            free(ptrs[i]);
    }

    printf("Heap allocator test ENDED\n");
}