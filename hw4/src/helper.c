#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "imprimer.h"
#include "sf_readline.h"
#include "helper.h"

int get_printer_index(char* name){
    for(int i=0;i<MAX_PRINTERS;i++){
        if(printer_array[i]==NULL){
            return i;
        }
        if(strcmp(printer_array[i]->name,name)==0){
            return -1;
        }
    }
    return -1;
}

void update_printer_status(PRINTER_STATUS status,char* name){
    if(status==PRINTER_DISABLED){
        printer_status_names[get_printer_index(name)]="disabled";
    }else if(status==PRINTER_IDLE){
        printer_status_names[get_printer_index(name)]="idle";
    }else{
        printer_status_names[get_printer_index(name)]="busy";
    }
}