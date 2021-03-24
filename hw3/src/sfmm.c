/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    pp=pp-8;
    if(pp==NULL){
        abort();
    }
    size_t header=GET(pp)->header;
    header=header|~(0x2);
    size_t size = GET_SIZE(pp);
    ((sf_block*)(pp))->header=PACK(size,0);
    ((sf_block*)(sf_mem_end()-16))->header=PACK(size,0);
    coalesce(pp);
    remove_block(pp);
    
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
