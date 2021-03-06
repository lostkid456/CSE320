#include<stdlib.h>
#include<stdio.h>

#include "helperfunctions.h"
#include "bdd.h"
#include "const.h"

#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

int index_counter=0;

int str_compare(char *str1,char *str2){
    while(*str1!='\0' && *str2!='\0'){
        if(*str1 != *str2){
            return 0;
        }
        str1++;
        str2++;
    }
    return 1; 
}


int str_to_int(char *str){
    int val=0;
    while(*str!='\0'){
        if(*str<48 || *str>57){
            return -1;   
        }
        val = val*10 +(*str-48);
        str++;
    }
    return val;
}

int simple_hash_function(int a, int b, int c, int d){
    int val=0;
    val=(5*(a+b+c))%d;
    return val;
}

int determine_powerof2(int a){
    int holder = a;
    int counter = 0;
    while (a > 1)
    {
        a = a >> 1;
        counter += 1;
    }
    int val = 1 << counter;
    if (holder > val)
    {
        return counter + 1;
    }
    return counter;
}

int recursive_from_raster(unsigned char *raster,int h, int w,int curr_h,int curr_w,int curr_x,int curr_y,int min_levels,int recursion_counter){
    //Stopping condition
    if(recursion_counter==min_levels){
        if(curr_x>(h-1) || curr_y>(w-1)){
            //printf("OUT OF BOUNDS %i %i %i\n",curr_x,curr_y,0);
            return 0;
        }else{
            //printf("IN BOUNDS %i %i %i\n",curr_x,curr_y,*raster);
            return *raster;
        }
    }
    if(recursion_counter%2==0){ //Split horizontally
        return bdd_lookup(min_levels-recursion_counter,recursive_from_raster(raster,h,w,curr_h/2,curr_w,curr_x,curr_y,min_levels,recursion_counter+1),
        recursive_from_raster(raster+(w*curr_h/2),h,w,curr_h/2,curr_w,curr_x+(curr_h/2),curr_y,min_levels,recursion_counter+1)); 
    }else{ //Split vertically
        return bdd_lookup(min_levels-recursion_counter,recursive_from_raster(raster,h,w,curr_h,curr_w/2,curr_x,curr_y,min_levels,recursion_counter+1),
        recursive_from_raster(raster+(curr_w/2),h,w,curr_h,curr_w/2,curr_x,curr_y+(curr_w/2),min_levels,recursion_counter+1));
    }
}

void find_serial(int value,FILE *out){
    int counter=0;
    while(counter<4){
        fputc(value,out);
        //printf(" %i\n",value);
        value=value>>8;
        counter+=1;
    }
}

void post_order(int *serial,BDD_NODE *node,FILE* out){
    if((*node).level==0 || (*node).left==(*node).right){
        if(*(bdd_index_map+(node-bdd_nodes))!=0){
            return;
        }else{
            fputc('@',out);
            fputc(node-bdd_nodes,out);
            //printf("%c %li\n",'@',node-bdd_nodes);
            *(bdd_index_map+(node-bdd_nodes))=*serial;
            *serial+=1;
            return;
        }
    }
    post_order(serial,LEFT(node,(*node).level),out);
    post_order(serial,RIGHT(node,(*node).level),out);
    if(*(bdd_index_map+(node-bdd_nodes))!=0){
        return;
    }else{
        *(bdd_index_map+(node-bdd_nodes))=*serial;
        (*serial)+=1;
        fputc((*node).level+'@',out);
        int left_serial=*(bdd_index_map+(*node).left);
        int right_serial=*(bdd_index_map+(*node).right);
        find_serial(left_serial,out);
        find_serial(right_serial,out);
    }
}

int rotate_helper(BDD_NODE *node,int level){
    if((*node).level==0){
        return node-bdd_nodes;
    }
    BDD_NODE *left_node=LEFT(node,(*node).level);
    BDD_NODE *right_node=RIGHT(node,(*node).level);
    int a=rotate_helper(LEFT(left_node,(*left_node).level),level-1);
    int b=rotate_helper(RIGHT(left_node,(*left_node).level),level-1);
    int c=rotate_helper(LEFT(right_node,(*right_node).level),level-1);
    int d=rotate_helper(RIGHT(right_node,(*right_node).level),level-1);
    int left_half=bdd_lookup((2*level)-1,b,d);
    int right_half=bdd_lookup((2*level)-1,a,c);
    return bdd_lookup(2*level,right_half,left_half);
}

int map_helper(BDD_NODE *node,unsigned char (*func)(unsigned char)){
    if((*node).level==0){
        return (*func)(node-bdd_nodes);
    }
    return bdd_lookup((*node).level,map_helper(LEFT(node,(*node).level),(*func)),map_helper(RIGHT(node,(*node).level),(*func)));
}

int zoom_in_helper(BDD_NODE *node,int zoom){
    if((*node).level==0){
        return node-bdd_nodes;
    }
    return bdd_lookup(((*node).level)+(2*zoom),zoom_in_helper(LEFT(node,(*node).level),zoom),zoom_in_helper(RIGHT(node,(*node).level),zoom));
}

int zoom_out_helper(BDD_NODE *node,int zoom){
    if((*node).level<=(2*zoom)){
        if(node-bdd_nodes==0){
            return 0;
        }else{
            return 255;
        }
    }
    return bdd_lookup(((*node).level)-(2*zoom),zoom_out_helper(LEFT(node,(*node).level),zoom),zoom_out_helper(RIGHT(node,(*node).level),zoom));
}

unsigned char negate(unsigned char val){
    return 255-val;
}
unsigned char threshold(unsigned char val){
    //printf("%i %i\n",global_options>>16,val);
    if(val<(global_options>>16)){
        return 0;
    }else{
        return 255;
    }
}



