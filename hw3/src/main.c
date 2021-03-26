#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    size_t sz_x = 64, sz_y = 8;
	int *x = sf_malloc(sz_x);
	int *y = sf_realloc(x, sz_y);
    *(y)=5;
    *(y+2)=10;
    *(y+32)=50;
    *(y+35)=10;
    sf_show_blocks();
    printf("\n");
    sf_show_free_lists();
    return EXIT_SUCCESS;
}
