#include <stdio.h>
#include "sfmm.h"
#include "helper.h"

int cnt=0;

//Initialize heap and free_list 
int sf_init()
{
    for(int i=0;i<NUM_FREE_LISTS;i++){
        sf_free_list_heads[i].body.links.next=&sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev=&sf_free_list_heads[i];
    }
    void *hp;
    if((hp=sf_mem_grow())!=NULL){
        // 8 bytes of padding 
        ((sf_block *)(hp))->header=0;
        hp+=8;
        // Prologue Header
        ((sf_block *)(hp))->header=PACK(32,1);
        hp+=32;
        // Block after prologue 
        ((sf_block *)(hp))->header=PACK(PAGE_SZ-48,0)|0x2;
        sf_free_list_heads[7].body.links.next=(sf_block *)(hp);
        sf_free_list_heads[7].body.links.prev=(sf_block *)(hp);
        ((sf_block *)(hp))->body.links.next=&sf_free_list_heads[7];
        ((sf_block *)(hp))->body.links.prev=&sf_free_list_heads[7];
        //Footer of block after prologue 
        hp=sf_mem_end()-16;
        ((sf_block *)(hp))->header=PACK(PAGE_SZ-48,0)|0x2;
        //Epilogue 
        hp=sf_mem_end()-8;
        ((sf_block *)(hp))->header=PACK(0,1);
        return 0;
    }
    return -1;
}

void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(PREV_BLKP(bp-8)) || (PREV_BLKP(bp-8)==bp);
    size_t next_alloc = GET_ALLOC(NEXT_BLKP(bp));
    size_t size = GET_SIZE(bp);
    if(prev_alloc && next_alloc){
        add_to_proper_index(bp);
        return bp;
    }else if(prev_alloc && !(next_alloc)){
        remove_block(NEXT_BLKP(bp));
        size+=GET_SIZE(NEXT_BLKP(bp));
        ((sf_block*)(bp))->header=PACK(size,0);
        ((sf_block*)(bp+size-8))->header=PACK(size,0);
    }else if(!(prev_alloc) && next_alloc){
        remove_block(PREV_BLKP(bp-8));
        size+=GET_SIZE(PREV_BLKP(bp-8));
        bp=PREV_BLKP(bp-8);
        ((sf_block*)(bp))->header=PACK(size,0);
        ((sf_block*)(bp+size-8))->header=PACK(size,0);
    }else{
        remove_block(PREV_BLKP(bp-8));
        remove_block(NEXT_BLKP(bp-8));
        size+=GET_SIZE(PREV_BLKP(bp-8))+ GET_SIZE(NEXT_BLKP(bp-8));
        bp=PREV_BLKP(bp-8);
        ((sf_block*)(bp))->header=PACK(size,0);
        ((sf_block*)(FTRP(bp-8)))->header=PACK(size,0);
    }
    add_to_proper_index(bp);
    return bp;
}

/*After mem_grow is called, we check whether there is enough space. If not then
we extend the heap with mem_grow. Else we initialize the new memory added.*/
void *init_extended_heap(size_t size,void *block){
    void *new_header=block-8;
    //Out of memory 
    if(block==NULL){
        return NULL;
    } 
    ((sf_block*)(new_header))->header=PACK(PAGE_SZ,0);
    void* hp=sf_mem_end()-16;
    ((sf_block*)(hp))->header=PACK(PAGE_SZ,0);
    hp=sf_mem_end()-8;
    ((sf_block*)(hp))->header=PACK(0,1);
    new_header=coalesce(new_header);
    if(GET_SIZE(new_header)<size){
        block=sf_mem_grow();
        if(block==NULL){
            return NULL;
        }
        return init_extended_heap(size,block);
    }
    return new_header;
}


//Place allocated block into heap 
void *place_block(void* block,size_t size){
    size_t mask=~(0xf);
    size_t header_size=(((sf_block*)(block))->header&mask);
    void *curr_free_block=find_segment(size);
    remove_block(block);
    if(header_size-size>=32){
        //Splitting block 
        //Allocated header
        ((sf_block*)(curr_free_block))->header=PACK(size,1)|0x2;
        block=NEXT_BLKP(curr_free_block);
        //Split free block header 
        ((sf_block*)(block))->header=PACK(header_size-size,0)|0x2;
        //Split free block footer
        ((sf_block*)(FTRP(block)))->header=PACK(header_size-size,0)|0x2;
        add_to_proper_index(block);
        return curr_free_block;
    }else{
        //No splitting 
        //Set header to allocated 
        ((sf_block*)(curr_free_block))->header=PACK(size,1)|0x2;
        return block;
    }
    return NULL;
}

//Free_list avaliability 
void *find_segment(size_t size){
    void* bp;
    size_t mask=~(0xf);
    for(int i=0;i<NUM_FREE_LISTS;i++){
        bp=&sf_free_list_heads[i];
        for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
            if(((((sf_block *)(bp))->header)&mask)>=size){
                    return bp;
            }
        }
    }
    return NULL;
}

//Free_list index based on avaliablity 
int segment_index(size_t size){
    void* bp;
    size_t mask=~(0xf);
    for(int i=0;i<NUM_FREE_LISTS;i++){
        if(i==0){
            if(size==32){
                bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return i;
                    }
                }
                continue;
            }
        }
        if(i==6){
            if(size>((1<<5)*32)){
                bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return i;
                    }
                }
                continue;
            }
        }
        if(i==7){
            bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return i;
                    }
                }
        }
        if(size>(32*(1<<(i-1))) &&  size<=(32*(1<<(i))) ){
            bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp))){
                        return i;
                    }
                }
        }
    }
    return 7;
}



//Remove from free_list based on avaliablility 
void remove_block(sf_block *block){
    size_t mask=~(0xf);
    size_t size=(block->header)&mask;
    sf_block *segment=&sf_free_list_heads[segment_index(size)];
    if(block==segment){
        printf("Can't remove sentinel node");
        return ;
    }
    block->body.links.next->body.links.prev=block->body.links.prev;
    block->body.links.prev->body.links.next=block->body.links.next;
    block->body.links.next=NULL;
    block->body.links.prev=NULL;
}


int proper_index(size_t size){
    for(int i=0;i<NUM_FREE_LISTS-1;i++){
        if(i==0){
            if(size==32){
                return i;
            }
            continue;
        }
        if(i==6){
            if(size>((1<<5)*32)){
                return i;
            }
            continue;
        }
        if( size>(32*(1<<(i-1))) &&  size<=(32*(1<<(i))) ){
            return i;
        }
    }
    return -1;
}

void add_to_proper_index(sf_block *block){
    size_t mask=~(0xf);
    size_t size=(block->header)&mask;
    sf_block *curr_segment=&sf_free_list_heads[proper_index(size)];
    block->body.links.next=curr_segment->body.links.next;
    block->body.links.prev=curr_segment;
    (curr_segment->body.links.next)->body.links.prev=block;
    curr_segment->body.links.next=block;
}

int ispowerof2(size_t size){
    int a=1;
    while(a<size){
        a=a<<1;
    }
    if(a!=size){
        return 1;
    }else{
        return 0;
    }
    return -1;
}

