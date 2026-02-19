#pragma once

#include "common.h"

void *memcpy(void *dest, const void *src, u32 n);

//cheat for allocating memory ,as i ahve not yet created a memory allocator

#define GPU_CACHED_BASE 0x40000000
#define GPU_UNCACHED_BASE 0xC0000000
#define GPU_MEM_BASE    GPU_UNCACHED_BASE

#define BUS_ADDRESS(addr) (((addr)& ~0xC0000000)|GPU_MEM_BASE)

#define PAGE_TEST_COUNT 1024

void *get_free_pages(int num_pages);
void *allocate_memory(int bytes);
void free_memory(void *base);

void page_stress_test();