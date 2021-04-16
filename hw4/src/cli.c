/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"
#include "helper.h"


int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if(in==NULL){
        return -1;
    }
    if(in==stdin && out==stdout){
        char* args;
        signal(SIGCHLD,sig_handler_child);
        sf_set_readline_signal_hook(sig_handler_parent);
        while(1){
            args=sf_readline("imp> ");
            int val=parse_inp(in,out,args);
            if(val==-1|| val==0){
                return val;
            }
        }
    }
    if(in==stdin && out!=stdout){
        char* args;
        signal(SIGCHLD,sig_handler_child);
        sf_set_readline_signal_hook(sig_handler_parent);
        while(1){
            args=sf_readline("");
            int val=parse_inp(in,out,args);
            if(val==-1|| val==0){
                return val;
            }
        }
    }
    if(in!=stdin){
        size_t lsize=0;
        char* bffer=0;
        ssize_t llen=0;
        signal(SIGCHLD,sig_handler_child);
        sf_set_readline_signal_hook(sig_handler_parent);
        while( (llen= getline(&bffer,&lsize,in)) !=-1){
            int len=strlen(bffer);
            if(bffer[len-1]=='\n'){
                bffer[len-1]='\0';
            }
            int val=parse_inp(in,out,bffer);
            bffer=NULL;
            if(val==-1){
                return val;
            }
        }
        return 0;
    }
    return 0;
}
