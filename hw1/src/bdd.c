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
    //printf("Level %i Left %i Right %i\n",level,left,right);
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
            int count=0;
            while(*bdd_hashtable_pointer!=NULL){
                if(hash_value+count>=BDD_NODES_MAX){
                    bdd_hashtable_pointer=bdd_hash_map;
                }
                if((*bdd_hashtable_pointer)->level == level && (*bdd_hashtable_pointer)->left==left && (*bdd_hashtable_pointer)-> right==right){
                    return *bdd_hashtable_pointer-bdd_nodes;
                }else{
                    bdd_hashtable_pointer+=1;
                    count+=1;
                }
            }
            BDD_NODE new_node={level,left,right};
            BDD_NODE *table_pointer=bdd_nodes;
            table_pointer+=index_counter+256;
            *table_pointer=new_node;
            *bdd_hashtable_pointer=table_pointer;
            index_counter+=1;
            //printf("New Node Created Index %li Level %i Left %i Right %i\n\n",table_pointer-bdd_nodes,level,left,right);
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
    int initialize_counter=0;
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
    int min_level=bdd_min_level(w,h);
    int x=0;
    int y=0;
    int counter=0;
    int root=recursive_from_raster(raster,h,w,smallest_h_dimension,smallest_w_dimension,x,y,min_level,counter);
    BDD_NODE *node=bdd_nodes+root;
    return node;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    int max_d_value=determine_powerof2(w)>determine_powerof2(h) ? determine_powerof2(w):determine_powerof2(h);
    int max_h=1<<max_d_value;
    int max_w=1<<max_d_value;
    for(int i=0;i<max_h;i++){
        for(int j=0;j<max_w;j++){
            *(raster+(i*w+j))=bdd_apply(node,i,j);
            //printf("bdd_to_raster %i i %i j %i\n",bdd_apply(node,i,j),i,j);
        }
    }
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    if(node==NULL){
        return -1;
    }
    int serial=1;
    post_order(&serial,node,out);
    return 0;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    //TO BE IMPLEMENTED
    int c=fgetc(in);
    int *node_arr=bdd_index_map+1;
    int counter=0;
    while(c!=EOF){
       if(c=='@'){
           c=fgetc(in);
           if(c<0 || c>255){
               return NULL;
           }else{
               *(node_arr+counter)=c;
               //printf("%i %i\n",64,*(node_arr+counter));
               counter+=1;
           }
       }else if(c>64 && c<96){
           int left_serial=0;
           for(int i=0;i<4;i++){
               int bit=fgetc(in);
               if(i==0){
                   left_serial+=bit;
               }
               if(i==1){
                   left_serial+=bit*(256);
               }
               if(i==2){
                   left_serial+=bit*(256*256);
               }
               if(i==3){
                   left_serial+=bit*(256*256*256);
               }
           }
           //printf("%i %i\n",c,left_serial);
           int right_serial=0;
           for(int i=0;i<4;i++){
               int bit=fgetc(in);
               if(i==0){
                   right_serial+=bit;
               }
               if(i==1){
                   right_serial+=bit*(256);
               }
               if(i==2){
                   right_serial+=bit*(256*256);
               }
               if(i==3){
                   right_serial+=bit*(256*256*256);
               }
           }
           //printf("%i %i %i\n",c,left_serial,right_serial);
           int index=bdd_lookup(c-64,*(node_arr+left_serial-1),*(node_arr+right_serial-1));
           *(node_arr+counter)=index;
           counter+=1;
       }else{
           return NULL;
       }
       c=fgetc(in);
    }
    //Returns the node with greatest serial number
    return bdd_nodes+*(node_arr+counter-1);
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    int apply_counter=0;
    int c_value=1;
    while(c_value>0){
        int level=(node->level);
        int bit_to_look;
        if(level==0){
            //printf("%i %i %li\n",r,c,node-bdd_nodes);
            return node-bdd_nodes;
        }
        if(level%2==0){
            bit_to_look=(level-2)/2;
            int mask=1<<bit_to_look;
            if((mask&r)>>bit_to_look){
                node=RIGHT(node,(*node).level);
            }else{
                node=LEFT(node,(*node).level);
            }
        }else{
            bit_to_look=(level-1)/2;
            if(bit_to_look==0){
                c_value=0;
            }
            int mask=1<<bit_to_look;
            if((mask&c)>>bit_to_look){
                node=RIGHT(node,(*node).level);
            }else{
                node=LEFT(node,(*node).level);
            }
        }
    }
    //printf("%i %i %li\n",r,c,node-bdd_nodes);
    return node-bdd_nodes;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    return map_helper(node,(*func))+bdd_nodes;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    int index=rotate_helper(node,level/2);
    return bdd_nodes+index;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    if(factor>0 && factor<=16){
        int zoom=1<<factor;
        int node_level=(*node).level;
        if(zoom+node<0 || zoom+node_level>BDD_LEVELS_MAX){
            return NULL;
        }else{
            return zoom_in_helper(node,zoom)+bdd_nodes;
        }
    }else if(factor>=240 && factor<=256){
        int zoom=1<<(factor-240);
        int node_level=(*node).level;
        if(zoom+node<0 || zoom+node_level>BDD_LEVELS_MAX){
            return NULL;
        }else{
            return zoom_out_helper(node,zoom)+bdd_nodes;
        }
    }else if(factor==0){
        return node;
    }else{
        return NULL;
    }
}
