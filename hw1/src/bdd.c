#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "debug.h"
#include "helperfunctions.h"

/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

/**
 * Determine the minimum number of levels required to cover a raster
 * with a specified width w and height h.
 *
 * @param w  The width of the raster to be covered.
 * @param h  The height of the raster to be covered.
 * @return  The least value l >=0 such that w <= 2^(l/2) and h <= 2^(l/2).
 */
int bdd_min_level(int w, int h){
    int l = determine_powerof2(w)>determine_powerof2(h) ? 2*determine_powerof2(w): 2*determine_powerof2(h) ;
    return l;
}

/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {
    // TO BE IMPLEMENTED
    printf("Level %i Left %i Right %i\n",level,left,right);
    if(level>BDD_LEVELS_MAX || level<0){
        return -1;
    }else{
        if(left==right){
            return left; 
        }
        if(left!=right){
            BDD_NODE **bdd_hashtable_pointer=bdd_hash_map; 
            int hash_value=simple_hash_function(level,left,right,BDD_NODES_MAX);
            bdd_hashtable_pointer+=hash_value;
            int counter=0;
            while(*bdd_hashtable_pointer!=NULL){
                if(hash_value+counter>=BDD_NODES_MAX){
                    bdd_hashtable_pointer=bdd_hash_map;
                }
                if((*bdd_hashtable_pointer)->level == level && (*bdd_hashtable_pointer)->left==left && (*bdd_hashtable_pointer)-> right==right){
                    return *bdd_hashtable_pointer-bdd_nodes;
                }else{
                    bdd_hashtable_pointer+=1;
                    counter+=1;
                }
            }
            BDD_NODE new_node={level,left,right};
            BDD_NODE *table_pointer=bdd_nodes;
            table_pointer+=index_counter+256;
            *table_pointer=new_node;
            *bdd_hashtable_pointer=table_pointer;
            index_counter+=1;
            printf("New Node Created Index %li Level %i Left %i Right %i\n\n",table_pointer-bdd_nodes,level,left,right);
            return table_pointer-bdd_nodes;
        }
    } 
    return -1;
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    if(w<0 || w>8192 || h<0 || h>8192){
        return NULL;
    }
    if(initialize_counter==0){
        BDD_NODE *hashmap_pointer=*bdd_hash_map;
        for(int i=0;i<BDD_NODES_MAX;i++){
            hashmap_pointer=NULL;
            hashmap_pointer++;
        }
        initialize_counter++;
    }
    int max_d_value= determine_powerof2(w)>determine_powerof2(h) ? determine_powerof2(w):determine_powerof2(h);
    int smallest_w_dimension= 1 << max_d_value;
    int smallest_h_dimension=1<<max_d_value;
    int w_dimension= 1<<max_d_value;
    int min_level=bdd_min_level(w,h);
    int x=0;
    int y=0;
    int counter=0;
    recursive_from_raster(raster,h,w,smallest_h_dimension,smallest_w_dimension,x,y,w_dimension,min_level,counter);
    BDD_NODE *node=bdd_nodes+BDD_NUM_LEAVES+index_counter-1;
    return node;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    if(node==NULL){
        return -1;
    }
    bdd_serialize(LEFT(node,(*node).level),out);
    bdd_serialize(RIGHT(node,(*node).right),out);
    int serial = bdd_lookup((*node).level,(*node).left,(*node).right);
    printf("%c %i",(*node).level+16,serial);
    return 0;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    return NULL;
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    return 0;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    return NULL;
}
