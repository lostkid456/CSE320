#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "imprimer.h"
#include "sf_readline.h"


//Define printer struct 
typedef struct printer{
    int id;
    char* name;
    char *type;
    PRINTER_STATUS status;
}PRINTER;

//Define job struct
typedef struct job{
    char* type;

    char* file;
}JOB;

//Array of printer pointers 
PRINTER *printer_array[MAX_PRINTERS];

//Array of job pointers
JOB *job_array[MAX_JOBS];



int get_printer_index(char* name);

void update_printer_status(PRINTER_STATUS status,char *name);


#endif