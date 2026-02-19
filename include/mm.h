#pragma once

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define GRAPH_START_MEMORY (2 * SECTION_SIZE)
#define LOW_MEMORY (32*1024*1024)

#define HIGH_MEMORY             	0x40000000
#define PAGING_MEMORY 			(HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES 			(PAGING_MEMORY/PAGE_SIZE)


#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned int n);


#endif