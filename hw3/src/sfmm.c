/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "sfmm.h"
#include "helper.h"


void *sf_malloc(size_t size) {
    if(cnt==0){
        if(sf_init()!=0){
            return NULL;
        }
    }
    cnt+=1;
    if(size==0){
        return NULL;
    }
    void *hp;
    if(size<=24){
        size=32;
    }else{
        size = (size + 8) % 16 == 0 ? size + 8 : (16 - ((size + 8) % 16)) + size + 8;
    }
    if((hp=find_segment(size))!=NULL){
        hp=place_block(hp,size);
        return (hp+8);
    }
    if((hp=sf_mem_grow())==NULL){
        sf_errno=12;
        return NULL;
    }
    if((hp=init_extended_heap(size,hp))==NULL){
        sf_errno=12;
        return NULL;
    }
    hp=place_block(hp,size);
    return (hp+8);
}

void sf_free(void *pp) {
    size_t bsize=GET_SIZE(pp-8);
    size_t allocator=GET_ALLOC(pp-8);
    sf_block* next_block=NEXT_BLKP(pp-8);
    if(pp==NULL||(bsize%16)||bsize<32||!(allocator)||((unsigned long)sf_mem_end()<(unsigned long)(next_block))){
        abort();
    }
    ((sf_block*)(NEXT_BLKP(pp-8)))->header=((sf_block*)(NEXT_BLKP(pp-8)))->header&(~0x2);
    ((sf_block*)(FTRP(NEXT_BLKP(pp-8))))->header=((sf_block*)(NEXT_BLKP(pp-8)))->header&(~0x2);
    ((sf_block*)(pp-8))->header=PACK(bsize,0)|0x2;
    ((sf_block*)(FTRP(pp-8)))->header=PACK(bsize,0)|0x2;
    sf_block* free_block= coalesce(pp-8);
    remove_block(free_block);
    add_to_proper_index(free_block);
}

void *sf_realloc(void *pp, size_t rsize) {
    size_t bsize=GET_SIZE(pp-8);
    size_t allocator=GET_ALLOC(pp-8);
    sf_block* next_block=NEXT_BLKP(pp-8);
    void *new_ptr;
    //Same checking conditions as free 
    if(pp==NULL||(bsize%16)||bsize<32||!(allocator)||((unsigned long)sf_mem_end()<(unsigned long)(next_block))){
        sf_errno=22;
        return NULL;
    }
    //If requested size is 0. Just free memory 
    if(rsize==0){
        sf_free(pp);
        return NULL;
    }
    //Size from header is the same as requested size
    if(bsize==rsize){
        return pp;
    }
    //If requested size is greater than header size 
    if(rsize>bsize){
        new_ptr=sf_malloc(rsize);
        memcpy(((sf_block*)(new_ptr))->body.payload,((sf_block*)(pp))->body.payload,rsize);
        sf_free(pp);
        return new_ptr;
    }
    size_t adjusted_size=(rsize + 8) % 16 == 0 ? rsize + 8 : (16 - ((rsize + 8) % 16)) + rsize + 8;
    if(adjusted_size<32){
        adjusted_size=32;
    }
    //Case 1 for realloc to smaller 
    //If there would not be a splinter if we split the block
    if(bsize-adjusted_size>=32){
        ((sf_block*)(pp-8))->header=PACK(adjusted_size,1)|0x2;
        ((sf_block*)(NEXT_BLKP(pp-8)))->header=PACK(bsize-adjusted_size,1)|0x2;
        sf_free(NEXT_BLKP(pp-8)+8);
        return pp;
    }
    //If there is a splinter if we split block 
    return pp;
}

void *sf_memalign(size_t size, size_t align) {
    if(size==0){
        return NULL;
    }
    if(align<size){
        sf_errno=22;
        return NULL;
    }
    return NULL;
}
