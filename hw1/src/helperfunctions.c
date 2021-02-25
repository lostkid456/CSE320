#include<stdlib.h>
#include<stdio.h>

#include "helperfunctions.h"
#include "bdd.h"

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

int recursive_from_raster(unsigned char *raster,int h, int w,int curr_h,int curr_w,int curr_x,int curr_y,int max_w,int min_levels,int recursion_counter){
    //Stopping condition
    if(recursion_counter==min_levels){
        if(curr_x>h || curr_y>w){
            return 0;
        }else{
            return *raster;
        }
    }
    if(recursion_counter%2==0){ //Split horizontally
        return bdd_lookup(min_levels-recursion_counter,recursive_from_raster(raster,h,w,curr_h/2,curr_w,curr_x,curr_y,max_w,min_levels,recursion_counter+1),
        recursive_from_raster(raster+(max_w*curr_h/2),h,w,curr_h/2,curr_w,curr_x+(curr_h/2),curr_y,max_w,min_levels,recursion_counter+1)); 
    }else{ //Split vertically
        return bdd_lookup(min_levels-recursion_counter,recursive_from_raster(raster,h,w,curr_h,curr_w/2,curr_x,curr_y,max_w,min_levels,recursion_counter+1),
        recursive_from_raster(raster+(curr_w/2),h,w,curr_h,curr_w/2,curr_x,curr_y+(curr_w/2),max_w,min_levels,recursion_counter+1));
    }
}

void find_serial(int value,FILE *out){
    int counter=0;
    while(counter<4){
        fputc(value,out);
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


