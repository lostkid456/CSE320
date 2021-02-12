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
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp=0,hp=0;
    if(img_read_pgm(in,&wp,&hp,raster_data,RASTER_SIZE_MAX)==0){
        int counter=0;
        for(int i=0;i<wp;i++){
            printf("\n");
            for(int j=0;j<hp;j++){
                if(raster_data[counter]>=0 && raster_data[counter]<=63){
                    printf(" ");
                }
                else if(raster_data[counter]>=64 && raster_data[counter]<=127){
                    printf(".");
                }else if(raster_data[counter]>=128 && raster_data[counter]<=191){
                    printf("*");
                }else if(raster_data[counter]>=192 && raster_data[counter]<=255){
                    printf("@");
                }else{
                    return -1;
                }
                counter++;
            }
        }
    }
    return 0;
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
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
                    if(output_counter==0){
                        global_options=global_options|0x20;                        
                    }
                }else{
                    global_options=0x00;
                    return -1;
                }
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
                        global_options=global_options|0x400;
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
                        global_options=global_options|0x400;
                        global_options=global_options|(-int_param<<4);
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
