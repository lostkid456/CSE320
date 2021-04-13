#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

//For forking and signals
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

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
    JOB_STATUS status;
    PRINTER *eligible_printers[MAX_PRINTERS];
    int eligible;
    char* file;
}JOB;

//Array of printer pointers 
PRINTER *printer_array[MAX_PRINTERS];

//Array of job pointers
JOB *job_array[MAX_JOBS];

int parse_inp(FILE *in,FILE *out,char* args);

int new_printer_index(char* name);

int find_printer_index(char *name);

void update_printer_status(PRINTER_STATUS status,char *name);

void update_job_status(JOB_STATUS status,int i);

int new_job_index(JOB *job);

int is_eligible_printer(JOB *job,PRINTER *printer);

void sig_handler_parent(int signal);

void sig_handler_child(int signal);

#endif