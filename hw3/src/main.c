#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    size_t sz_u = 200, sz_v = 300, sz_w = 200, sz_x = 500, sz_y = 200, sz_z = 700;
	void *u = sf_malloc(sz_u);
	/* void *v = */ sf_malloc(sz_v);
	void *w = sf_malloc(sz_w);
	/* void *x = */ sf_malloc(sz_x);
	void *y = sf_malloc(sz_y);
	/* void *z = */ sf_malloc(sz_z);

	sf_free(u);
	sf_free(w);
	sf_free(y);

    // char * ptr1 = sf_malloc(1);
    // *(ptr1) = 'A';
    
    // sf_show_blocks();   
    // printf("\n"); 
    
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

    // int *x = sf_malloc(32704);
    // *(x)=2;

    // sf_show_blocks();
    // printf("\n");   

    // sf_free(x); 

    // sf_free(ptr1);

    // sf_free(ptr3);

    sf_show_blocks();
    printf("\n");

    sf_show_free_lists();  
    printf("\n"); 

    return EXIT_SUCCESS;
}
