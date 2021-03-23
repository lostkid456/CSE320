#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include "sfmm.h"

int cnt;

#define PACK(size, alloc)  ((size) | (alloc))
#define GET(p)           ((sf_block *)(p))


#define GET_SIZE(p)  (GET(p)->header & ~(0xf))
#define GET_ALLOC(p) (GET(p)->header & 0x1)
#define GET_PREVALLOC(p) (GET(p->header) & 0x2)

#define HDRP(bp)       ( (void *)(bp))
#define FTRP(bp)       ( (void *)(bp) + GET_SIZE(HDRP(bp)) - 8 )

#define NEXT_BLKP(bp)  ( (void *)(bp) + GET_SIZE(((void *)(bp) )) )
#define PREV_BLKP(bp)  ( (void *)(bp) - GET_SIZE(((void *)(bp) )) + 8  )

int sf_init(); 

void *init_extended_heap(size_t size,void *block);

void *find_free_region();

void* place_block(void* block,size_t size);

void *find_segment(size_t size);

int segment_index(size_t size);

void remove_block(sf_block *block);

void add_block(sf_block *block);

#endif