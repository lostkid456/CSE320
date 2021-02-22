#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    if(global_options == 0x31){
        pgm_to_ascii(stdin,stdout);
    }
    if(global_options == 0x21){
        // unsigned char input1[] = {4, 8 ,4, 8};
        // bdd_from_raster(2,2,input1);
        // unsigned char input2[] = {4,2,12,255};
        // bdd_from_raster(2,2,input2);
        // unsigned char input3[] = {4,2, 36, 49};
        // bdd_from_raster(2,2,input3);
        // unsigned char input4[] = {4,2, 4,8};
        // bdd_from_raster(2,2,input4);
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
