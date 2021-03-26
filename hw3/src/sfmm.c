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
    if(pp==NULL||(GET_SIZE(pp-8)%16)||GET_SIZE(pp-8)<32||!(GET_ALLOC(pp-8))||((unsigned long)sf_mem_end()<(unsigned long)(NEXT_BLKP(pp-8)))||((unsigned long)(pp))%16){
        abort();
    }
    size_t bsize=GET_SIZE(pp-8);
    ((sf_block*)(NEXT_BLKP(pp-8)))->header=((sf_block*)(NEXT_BLKP(pp-8)))->header&(~0x2);
    ((sf_block*)(FTRP(NEXT_BLKP(pp-8))))->header=((sf_block*)(NEXT_BLKP(pp-8)))->header&(~0x2);
    ((sf_block*)(pp-8))->header=PACK(bsize,0)|0x2;
    ((sf_block*)(FTRP(pp-8)))->header=PACK(bsize,0)|0x2;
    sf_block* free_block= coalesce(pp-8);
    remove_block(free_block);
    add_to_proper_index(free_block);
}

void *sf_realloc(void *pp, size_t rsize) {
    //Same checking conditions as free 
    if(pp==NULL||(GET_SIZE(pp-8)%16)||GET_SIZE(pp-8)<32||!(GET_ALLOC(pp-8))||((unsigned long)sf_mem_end()<(unsigned long)(NEXT_BLKP(pp-8)))||((unsigned long)(pp))%16){
        sf_errno=22;
        return NULL;
    }
    size_t bsize=GET_SIZE(pp-8);
    void *new_ptr;
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
    if(align<32 || ispowerof2(align)){
        sf_errno=22;
        return NULL;
    }
    if(size==0){
        return NULL;
    }
    void *huge_block=sf_malloc(size+align+32+8);
    size_t huge_block_size=GET_SIZE(huge_block-8);
    size_t padding=(align-(((unsigned long)(huge_block))%align))%align;
    //NEED TO ALIGN THE PAYLOAD
    if(padding>0){
        if(padding<32){
            padding+=align;
        }
        //Split first time 
        ((sf_block*)(huge_block-8))->header=PACK(padding,0);
        ((sf_block*)(FTRP(huge_block-8)))->header=PACK(padding,0);
        ((sf_block*)(NEXT_BLKP(huge_block-8)))->header=PACK(huge_block_size-padding,1);
        coalesce(huge_block-8);
        huge_block=NEXT_BLKP(huge_block-8);
        if(size<=24){
            size=32;
        }else{
            size = (size + 8) % 16 == 0 ? size + 8 : (16 - ((size + 8) % 16)) + size + 8;
        }
        if(GET_SIZE(huge_block)-size>=32){
            //Still can split another time 
            size_t osize=GET_SIZE(huge_block);
            ((sf_block*)(huge_block))->header=PACK(size,1);
            void *n_huge_block=NEXT_BLKP(huge_block);
            ((sf_block*)(n_huge_block))->header=PACK(osize-size,0)|0x2;
            ((sf_block*)(FTRP(n_huge_block)))->header=PACK(osize-size,0)|0x2;
            coalesce(n_huge_block);
        }
        sf_show_free_lists();
        return huge_block+8;
    }
    //PAYLOAD ALREADY ALIGNED
    else{
        if(size<=24){
            size=32;
        }else{
            size = (size + 8) % 16 == 0 ? size + 8 : (16 - ((size + 8) % 16)) + size + 8;
        }
        if(GET_SIZE(huge_block-8)-size>=32){
            ((sf_block*)(huge_block-8))->header=PACK(size,1);
            void *n_huge_block=NEXT_BLKP(huge_block-8);
            ((sf_block*)(n_huge_block))->header=PACK(huge_block_size-size,0)|0x2;
            ((sf_block*)(FTRP(n_huge_block)))->header=PACK(huge_block_size-size,0)|0x2;
            coalesce(n_huge_block);
        }
        sf_show_free_lists();
        return huge_block;
    }
    return huge_block;
}
