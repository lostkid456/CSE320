#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    char * ptr1 = sf_malloc(50 * sizeof(double));
    *(ptr1) = 'A';
    
    sf_show_blocks();
    printf("\n");
    
    char * ptr2 = sf_malloc(78 * sizeof(double));
    *(ptr2) = 'A';
    
    sf_show_blocks();
    printf("\n");
    
    char * ptr3 = sf_malloc(1 * sizeof(double));
    *(ptr3) = 'A';
    
    sf_show_blocks();
    printf("\n");
    
    ptr2 = sf_realloc(ptr2, 500);
    sf_show_blocks();
    printf("\n");
    
    ptr3 = sf_realloc(ptr3, 12); // Should be just using the same block
    sf_show_blocks();
    printf("\n");
    
    char * ptr4 = sf_malloc(7000); // Allocate 7008 bytes 
    *(ptr4) = 'A'; // Should only have 48 bytes left
    sf_show_blocks();
    printf("\n");
    
    ptr3 = sf_realloc(ptr3, 10); // Should again use the same block now that it is not in fron tof wilderness
    sf_show_blocks();
    printf("\n");
    
    
    char * ptr5 = sf_realloc(ptr2, 200); // Reallocing with merging
    *(ptr5) = 'A';
    sf_show_blocks();
    printf("\n");
    
    char * ptr6 = sf_realloc(ptr4, 2300); //Reallocing with merging with the wilderness
    *(ptr6) = 'A';
    sf_show_blocks();
    printf("\n");
    
    char * ptrA = sf_memalign(50, 128); // Want a 50 byte block that is 128 aligned
    *(ptrA) = 'A';
    sf_show_blocks();
    printf("\n");
    
    char * ptrB = sf_memalign(50, 32); // Want a 50 byte block that is 32 byte aligned
    *(ptrB) = 'A';
    sf_show_blocks();
    printf("\n");
    
    sf_show_free_lists();

    return EXIT_SUCCESS;
}
