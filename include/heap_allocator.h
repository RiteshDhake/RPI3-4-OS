#pragma once

#include "common.h"

typedef struct heap_block {
    size_t size;
    struct heap_block *next;
    int free;
} heap_block_t;

void heap_init();
void *malloc(size_t size);      
void free(void *ptr);