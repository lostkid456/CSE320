#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // char * ptr1 = sf_malloc(1);
    // *(ptr1) = 'A';
    
    // sf_show_blocks();   
    // printf("\n"); 

    //sf_free(ptr1);
    
    // char * ptr2 = sf_malloc(1);
    // *(ptr2) = 'B';
    
    // sf_show_blocks();    
    // printf("\n"); 
    
    // int * ptr3 = sf_malloc(2020 * sizeof(int));
    // *(ptr3 + 0) = 1;
    // *(ptr3 + 1) = 69;
    // *(ptr3 + 2) = 80;
    // *(ptr3 + 23) = 69;

    // sf_show_blocks();  
    // printf("\n"); 
    
    // char *ptr4 = sf_malloc(8168);
    // *(ptr4) = 'Y';
    
    // sf_show_blocks();  
    // printf("\n"); 
    
    // int * ptr5 = sf_malloc(9000);
    // if(ptr5 != NULL)
    // {
    //     *(ptr5) = 'A';
    // }
    
    // sf_show_blocks();  
    // printf("\n"); 

    // int *a=sf_malloc(524288);
    // if(a!=NULL){
    //     printf("Not null");
    // }

    int *x = sf_malloc(32704);
    *(x)=2;

    sf_show_blocks();
    printf("\n");    

    sf_show_free_lists();  
    printf("\n"); 

    return EXIT_SUCCESS;
}
