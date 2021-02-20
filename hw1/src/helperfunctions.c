#include<stdlib.h>
#include<stdio.h>

#include "helperfunctions.h"
#include "bdd.h"

int index_counter=0;

int initalize_counter=0;


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
        if(curr_x>h || curr_y>w){
            return 0;
        }else{
            return *raster;
        }
    }
    if(recursion_counter%2==0){ //Split horizontally
        recursive_from_raster(raster,h,w,curr_h/2,curr_w,curr_x,curr_y,min_levels,recursion_counter++); 
        recursive_from_raster(raster+((curr_x + h/2) * w + curr_y),h,w,curr_h/2,curr_w,curr_x+(curr_h/2),curr_y,min_levels,recursion_counter++); 
    }else{ //Split vertically
        recursive_from_raster(raster,h,w,curr_h,curr_w/2,curr_x,curr_y,min_levels,recursion_counter++); 
        recursive_from_raster(raster+(curr_x * w/2 + curr_y + w/2),h,w,curr_h,curr_w/2,curr_x,curr_y+(curr_w/2),min_levels,recursion_counter++);
    }
    return 0;
}


