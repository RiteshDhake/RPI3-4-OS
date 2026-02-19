#pragma once

#include "common.h"

#define HEAP_TEST_COUNT 2048

typedef struct heap_block {
    size_t size;
    struct heap_block *next;
    int free;
} heap_block_t;

void heap_init();
void *malloc(size_t size);      
void free(void *ptr);
static heap_block_t* heap_expand();
void heap_stress_test();