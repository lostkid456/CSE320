#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include "sfmm.h"

int cnt;

#define PACK(size, alloc)  ((size) | (alloc))
#define GET(p)           ((sf_block *)(p))


#define GET_SIZE(p)  (GET(p)->header & ~(0xf))
#define GET_ALLOC(p) (GET(p)->header & 0x1)
#define GET_PREVALLOC(p) (GET(p)->header & 0x2)

#define FTRP(p) ( (void*)(p) + GET_SIZE(((void *)(p) )) -8 )

#define NEXT_BLKP(bp)  ( (void *)(bp) + GET_SIZE(((void *)(bp) )) )
#define PREV_BLKP(bp)  ( (void *)(bp) - GET_SIZE(((void *)(bp) )) + 8  )

int sf_init(); 

void *coalesce(void *bp);

void *init_extended_heap(size_t size,void *block);

void* place_block(void* block,size_t size);

void *find_segment(size_t size);

int segment_index(size_t size);

void remove_block(sf_block *block);

int proper_index(size_t size);

void add_to_proper_index(sf_block *block);

int ispowerof2(size_t size);


#endif