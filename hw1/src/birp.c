/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"
#include "helperfunctions.h"

int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    if(img_read_pgm(in,&wp,&hp,raster_data,RASTER_SIZE_MAX)==0){
        BDD_NODE *node=bdd_from_raster(wp,hp,raster_data);
        img_write_birp(node,wp,hp,stdout);
        return 0;
    }else{
        return -1;
    }
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    BDD_NODE *node=img_read_birp(in,&wp,&hp);
    bdd_to_raster(node,wp,hp,raster_data);
    img_write_pgm(raster_data,wp,hp,out);
    return 0;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    BDD_NODE *node=img_read_birp(in,&wp,&hp);
    int *index_map_pointer=bdd_index_map;
    for(int i=0;i<BDD_NODES_MAX;i++){
        *(index_map_pointer)=0;
        index_map_pointer++;
    }
    if((global_options & 0xf00)>>8==0x4){
        BDD_NODE *rotated=bdd_rotate(node,(*node).level);
        img_write_birp(rotated,wp,hp,out);
        return 0;
    }
    if((global_options & 0xf00)>>8==0x3){
        int zoom=global_options>>16;
        BDD_NODE *zoomed=bdd_zoom(node,0,zoom);
        if(zoom>16){
            img_write_birp(zoomed,wp>>(zoom-240),hp>>(zoom-240),out);
        }else{
            img_write_birp(zoomed,wp<<zoom,hp<<zoom,out);
        }
        return 0;
    }
    if((global_options & 0xf00)>>8==0x2){
        unsigned char (*func)(unsigned char);
        func=&threshold;
        BDD_NODE *thresholded=bdd_map(node,func);
        img_write_birp(thresholded,wp,hp,out);
        return 0;
    }
    if((global_options & 0xf00)>>8==0x1){
        unsigned char (*func)(unsigned char);
        func=&negate;
        BDD_NODE *negated=bdd_map(node,func);
        img_write_birp(negated,wp,hp,out);
        return 0;
    }
    if((global_options & 0xf00)>>8==0x0){
        img_write_birp(node,wp,hp,out);
        return 0;
    }
    return 0;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    if(img_read_pgm(in,&wp,&hp,raster_data,RASTER_SIZE_MAX)==0){
        unsigned char *arr_p=raster_data;
        for(int i=0;i<hp;i++){
            for(int j=0;j<wp;j++){
               // printf("%i %i %i\n",i,j,*arr_p);
                if(*arr_p>=0 && *arr_p<=63){
                    printf(" ");
                }
                else if(*arr_p>=64 && *arr_p<=127){
                    printf(".");
                }else if(*arr_p>=128 && *arr_p<=191){
                    printf("*");
                }else if(*arr_p>=192 && *arr_p<=255){
                    printf("@");
                }else{
                    return -1;
                }
                arr_p++;
            }
            printf("\n");
        }
        printf("\n");
        return 0;
    }else{
        return -1;
    }
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    BDD_NODE *node=img_read_birp(in,&wp,&hp);
    bdd_to_raster(node,wp,hp,raster_data);
    unsigned char *arr_p=raster_data;
        for(int i=0;i<hp;i++){
            for(int j=0;j<wp;j++){
                //printf("%i %i %i\n",i,j,*arr_p);
                if(*arr_p>=0 && *arr_p<=63){
                    printf(" ");
                }
                else if(*arr_p>=64 && *arr_p<=127){
                    printf(".");
                }else if(*arr_p>=128 && *arr_p<=191){
                    printf("*");
                }else if(*arr_p>=192 && *arr_p<=255){
                    printf("@");
                }else{
                    return -1;
                }
                arr_p++;
            }
            printf("\n");
        }
        printf("\n");
        return 0;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    // TO BE IMPLEMENTED
    global_options=0x00;
    argv++;
    if(argc==0){
        return -1;
    }
    if(argc==1){
        global_options=global_options|0x22;
    }
    if (argc>=2){
        int leading_flag_counter=0;
        int optional_flag_counter=0; 
        int birp_selected=0;
        int input_counter=0;
        int output_counter=0;
        char *flag;
        char *param;
        for(int i=1;i<argc;i++){
            flag="-h";
            if(str_compare(*argv,flag)){
                if(optional_flag_counter==0){
                    global_options = (global_options | 0x80000000);
                    break;
                }else{
                    global_options=0x00;
                    return -1;
                }
            }
            flag="-i";
            if(str_compare(*argv,flag)){
                if(optional_flag_counter==0){
                    argv++;
                    i++;
                    if(i>=argc){
                        return -1; 
                    }
                    if(!(str_compare(*argv,"pgm") || str_compare(*argv,"birp"))){
                        return -1;
                    }
                    param="pgm";
                    if(str_compare(*argv,param)){
                        global_options = global_options | 0x1;
                    }
                    param="birp";
                    if(str_compare(*argv,param)){
                        birp_selected+=1;
                        global_options = global_options | 0x2;
                    }
                    input_counter+=1;
                    leading_flag_counter+=1;
                }else{
                    global_options=0x00;
                    return -1;
                }
            }
            if(input_counter==1 && output_counter==0){
                global_options = global_options | 0x20;
            }
            flag="-o";
            if(str_compare(*argv,flag)){
                if(optional_flag_counter==0){
                    argv++;
                    i++;
                    if(i>=argc){
                        return -1; 
                    }
                    if(!(str_compare(*argv,"pgm") || str_compare(*argv,"birp") || str_compare(*argv,"ascii"))){
                        return -1;
                    }
                    param="pgm";
                    if(str_compare(*argv,param)){
                        global_options = global_options | 0x10;
                    }
                    param="birp";
                    if(str_compare(*argv,param)){
                        birp_selected+=1;
                        global_options = global_options | 0x20;
                    }
                    param="ascii";
                    if(str_compare(*argv,param)){
                        global_options = global_options | 0x30 ;
                    }
                    output_counter+=1;
                    leading_flag_counter+=1;
                    if(input_counter==0){
                        global_options=global_options|0x2;                        
                    }
                }else{
                    global_options=0x00;
                    return -1;
                }
            }
            flag="-n";
            if(str_compare(*argv,flag)){
                if(leading_flag_counter<1){
                    birp_selected=2;
                }
                if(birp_selected==2){
                    optional_flag_counter+=1;
                    global_options=global_options|0x100;
                    if(input_counter==0){
                        global_options=global_options|0x2;
                    }
                    if(output_counter==0){
                        global_options=global_options|0x20;
                    }
                }else{
                    return -1;
                }   
            }
            flag="-t";
            if(str_compare(*argv,flag)){
                if(leading_flag_counter<1){
                    birp_selected=2;
                }
                if(birp_selected==2){
                    argv++;
                    i++;
                    if(i>=argc){
                        return -1; 
                    }
                    int int_param=str_to_int(*argv);
                    if(int_param>=0 && int_param<=255){
                        global_options=global_options|0x200;
                        global_options=global_options|(int_param<<16);
                        optional_flag_counter+=1;
                        if(input_counter==0){
                            global_options=global_options|0x2;
                        }
                        if(output_counter==0){
                            global_options=global_options|0x20;
                        }
                    }else{
                        return -1;
                    }
                }else{
                    return -1;
                } 
            }
            flag="-r";
            if(str_compare(*argv,flag)){
                if(leading_flag_counter<1){
                    birp_selected=2;
                }
                if(birp_selected==2){
                    global_options=global_options|0x400;
                    optional_flag_counter+=1;
                    if(input_counter==0){
                        global_options=global_options|0x2;
                    }
                    if(output_counter==0){
                        global_options=global_options|0x20;                        
                    }
                }else{
                    return -1;
                }
            }
            flag="-z";
            if(str_compare(*argv,flag)){
                if(leading_flag_counter<1){
                    birp_selected=2;
                }
                if(birp_selected==2){
                    argv++;
                    i++;
                    if(i>=argc){
                        return -1; 
                    }
                    int int_param=str_to_int(*argv);
                    if(int_param>=0 && int_param<=16){
                        global_options=global_options|0x300;
                        global_options=global_options|((256-int_param)<<16);
                        optional_flag_counter+=1;
                        if(input_counter==0){
                            global_options=global_options|0x2;
                        }
                        if(output_counter==0){
                            global_options=global_options|0x20;                        
                        }
                    }else{
                        return -1;
                    }
                }else{
                    return -1; 
                } 
            }
            flag="-Z";
            if(str_compare(*argv,flag)){
                if(leading_flag_counter<1){
                    birp_selected=2;
                }
                if(birp_selected==2){
                    argv++;
                    i++;
                    if(i>=argc){
                        return -1; 
                    }
                    int int_param=str_to_int(*argv);
                    if(int_param>=0 &&  int_param<=16){
                        global_options=global_options|0x300;
                        global_options=global_options|(int_param<<16);
                        optional_flag_counter+=1;
                        if(input_counter==0){
                            global_options=global_options|0x2;
                        }
                        if(output_counter==0){
                            global_options=global_options|0x20;                        
                        }
                    }else{
                        return -1;
                    }
                }else{
                    return -1; 
                } 
            }
            argv++;
        }
    }
    return 0;
}
