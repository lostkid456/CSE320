#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//For forking
#include <unistd.h>
#include <sys/types.h>

#include "imprimer.h"
#include "sf_readline.h"
#include "helper.h"

int new_printer_index(char* name){
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

int find_printer_index(char *name){
    for(int i=0;i<MAX_PRINTERS;i++){
        if(printer_array[i]!=NULL){
            if(strcmp(printer_array[i]->name,name)==0){
                return i;
            }
        }
    }
    return -1;
}

void update_printer_status(PRINTER_STATUS status,char* name){
    if(status==PRINTER_DISABLED){
        printer_status_names[new_printer_index(name)]="disabled";
    }else if(status==PRINTER_IDLE){
        printer_status_names[new_printer_index(name)]="idle";
    }else{
        printer_status_names[new_printer_index(name)]="busy";
    }
}

void update_job_status(JOB_STATUS status,int i){
    if(status==JOB_CREATED){
        job_status_names[i]="created";
    }else if(status==JOB_RUNNING){
        job_status_names[i]="busy";
    }else if(status==JOB_PAUSED){
        job_status_names[i]="pause";
    }else if(status==JOB_FINISHED){
        job_status_names[i]="finished";
    }else if(status==JOB_ABORTED){
        job_status_names[i]="aborted";
    }else{
        job_status_names[i]="deleted";
    }
}

int new_job_index(JOB *job){
    for(int i=0;i<MAX_JOBS;i++){
        if(job_array[i]==NULL){
            return i;
        }
    }
    return -1;
}

int is_eligible_printer(JOB *job,PRINTER *printer){
    if(job->eligible==0){
        return 0;
    }
    for(int i=0;i<job->eligible;i++){
        if(strcmp(job->eligible_printers[i]->name,printer->name)==0){
            return 0;
        }
    }
    return -1;
}

void sig_handler_parent(int signal){

}

void sig_handler_child(int signal){

}

