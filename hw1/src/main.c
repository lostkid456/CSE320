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
        return 0;
    }
    if(global_options == 0x21){
    //    unsigned char input2[] = {1,2,3,4,5,6,7,8,9};
    //    bdd_from_raster(3, 3, input2);
       pgm_to_birp(stdin,stdout);
       return 0;
    }
    if(global_options == 0x12){
        birp_to_pgm(stdin,stdout);
        return 0;
    }
    if(global_options == 0x32){
        birp_to_ascii(stdin,stdout);
        return 0;
    }
    if((global_options & 0xff) == 0x22){
        birp_to_birp(stdin,stdout);
        return 0;
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
