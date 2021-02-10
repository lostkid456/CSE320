#include<stdlib.h>
#include<stdio.h>

#include "helperfunctions.h"


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


