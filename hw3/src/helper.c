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
    if((hp=sf_mem_grow())!=0){
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

/*After mem_grow is called, we check whether there is enough space. If not then
we extend the heap with mem_grow. Else we initialize the new memory added.*/
void *init_extended_heap(size_t size,void *block){
    void *new_header=block-8;
    void *old_footer=new_header-8;
    size_t mask=~(0xf);
    size_t previous_size=(((sf_block*)(old_footer))->header)&mask;
    //Out of memory 
    if(block==NULL){
        return NULL;
    }
    //If not enough space 
    if(((previous_size+PAGE_SZ-16)-size)<32){
        return init_extended_heap(size,sf_mem_grow());
    }
    // Old Page is FULL 
    if(GET_ALLOC(old_footer)){
        ((sf_block*)(new_header))->header=PACK(PAGE_SZ,0);
        ((sf_block*)(sf_mem_end()-16))->header=PACK(PAGE_SZ,0);
        ((sf_block*)(sf_mem_end()-8))->header=PACK(0,1);
    }else{
        //Old page not full, coalesce the previous header to the new header.
        //Must also remove the old header from the free_list  and add the new _header to the free_list  
        ((sf_block*)(new_header))->header=PACK(PAGE_SZ+previous_size,0);
        remove_block((sf_block*)(PREV_BLKP(old_footer)));
        add_block(new_header);
        void* hp=sf_mem_end()-16;
        ((sf_block*)(hp))->header=PACK(PAGE_SZ+previous_size,0);
        hp=sf_mem_end()-8;
        ((sf_block*)(hp))->header=PACK(0,1);
    }
    return new_header;
}

//Find free_region for malloc 
void *find_free_region(){
    void *hp=sf_mem_start()+8;
    while(GET_ALLOC(hp)){
        hp=NEXT_BLKP(hp);
    }
    return hp;
}


//Place allocated block into heap 
void *place_block(void* block,size_t size){
    size_t mask=~(0xf);
    size_t header_size=(((sf_block*)(block))->header&mask);
    remove_block(block);
    void *curr_free_block=find_free_region();
    size_t prev_size=(((sf_block*)(curr_free_block))->header)&mask;
    printf("%li\n",prev_size);
    if(header_size-size>=32){
        //Splitting block 
        //Allocated header
        printf("%p\n",curr_free_block);
        ((sf_block*)(curr_free_block))->header=PACK(size,1)|0x2;
        block=NEXT_BLKP(curr_free_block);
        printf("%p\n",block);
        //Split free block header 
        ((sf_block*)(block))->header=PACK(header_size-size,0)|0x2;
        //Split free block footer
        ((sf_block*)(FTRP(block)))->header=PACK(header_size-size,0)|0x2;
        printf("%p\n",FTRP(block));
        add_block(block);
        return curr_free_block;
    }else{
        //No splitting 
        //Set header to allocated 
        ((sf_block*)(block))->header=PACK(size,1)|0x2;
        return block;
    }
    return NULL;
}

//Free_list avaliability 
void *find_segment(size_t size){
    void* bp;
    size_t mask=~(0xf);
    for(int i=0;i<NUM_FREE_LISTS;i++){
        if(i==0){
            if(size==32){
                bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return bp;
                    }
                }
            }
        }
        if(i==6){
            if(size>(1<<5)*32){
                bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return bp;
                    }
                }
            }
        }
        if(i==7){
            bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return bp;
                    }
                }
        }
        if(size>(32*(1<<(i-1))) &&  size<=(32*(1<<(i))) ){
            bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size){
                        return bp;
                    }
                }
        }
    }
    return NULL;
}

//Free_list index
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
            }
        }
        if(i==6){
            if(size>(1<<5)*32){
                bp=&sf_free_list_heads[i];
                for(bp=((sf_block*)(bp))->body.links.next;((((sf_block *)(bp))->header)&mask)>0;bp=((sf_block*)(bp))->body.links.next){
                    if(((((sf_block *)(bp))->header)&mask)>=size && !(GET_ALLOC(bp)) ){
                        return i;
                    }
                }
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



//Remove from free_list
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

//Add to free_list 
void add_block(sf_block *block){
    size_t mask=~(0xf);
    size_t size=(block->header)&mask;
    sf_block *curr_segment=&sf_free_list_heads[segment_index(size)];
    //Last block in free_list
    curr_segment=curr_segment->body.links.prev;
    block->body.links.next=curr_segment->body.links.next;
    block->body.links.prev=curr_segment;
    curr_segment->body.links.next->body.links.prev=block;
    curr_segment->body.links.next=block;
}

