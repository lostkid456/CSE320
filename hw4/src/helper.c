#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//For forking and signals
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#include "imprimer.h"
#include "sf_readline.h"
#include "conversions.h"
#include "helper.h"

int parse_inp(FILE *in,FILE *out,char* args){
    char *args_token=strtok(args," ");
    if(args_token==NULL){
        free(args);  
        return 1; 
    }
    //EOF encountered
    if(strcmp(args_token,"\n")==0){
        free(args);
        return -1;
    }
    //Miscellaneous commands
    if(strcmp(args_token,"help")==0){
        if(strtok(NULL," ")!=NULL){
            free(args);
            fprintf(out,"Help does not require any additional arguments\n");
            sf_cmd_error("Too many args");   
            return 1;     
        }
        fprintf(out,"Commands are: help, quit, type, printer, conversion, printers, jobs, print, cancel, disable, enable, pause and resume\n");
        free(args);
        sf_cmd_ok();
        return 1;
    }
    else if(strcmp(args_token,"quit")==0){
        if(strtok(NULL," ")!=NULL){
            free(args);
            fprintf(out,"Quit does not require any additional arguments\n");
            sf_cmd_error("Too many args");
            return 1;
        }
        free(args);
        for(int i=0;i<MAX_PRINTERS;i++){
            if(printer_array[i]!=NULL){
                free(printer_array[i]->name);
                free(printer_array[i]);
            }
        }
        for(int i=0;i<MAX_JOBS;i++){
            if(job_array[i]!=NULL){
                free(job_array[i]->file);
                free(job_array[i]);
            }
        }
        sf_cmd_ok();
        return -1;
    }
    //Configuration commands
    else if(strcmp(args_token,"type")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char *file_type=args_token;
            if(strtok(NULL," ")!=NULL){
                fprintf(out,"Type only requires one additional argument\n");
                sf_cmd_error("Too many args");
                free(args);
                return 1;
            }
            define_type(file_type);
            free(args);
            sf_cmd_ok();
            return 1;
        }
    }else if(strcmp(args_token,"printer")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char *printer_name=args_token;
            if((args_token=strtok(NULL," "))!=NULL){
                char *file_type=args_token;
                if((args_token=strtok(NULL," "))!=NULL){
                    free(args);
                    fprintf(out,"Printer only requires two additional argument\n");
                    sf_cmd_error("Too many args");
                    return 1;
                }
                if(find_type(file_type)==NULL){
                    free(args);
                    fprintf(out,"Type is not defined\n");
                    sf_cmd_error("Can't find type");
                    return 1;
                }
                int index=new_printer_index(printer_name);
                if(index!=-1){
                    update_printer_status(PRINTER_DISABLED,printer_name);
                    PRINTER *new_printer=malloc(sizeof(PRINTER));
                    new_printer->id=index;
                    char* name=malloc(strlen(printer_name)+1);
                    memcpy(name,printer_name,strlen(printer_name)+1);
                    new_printer->name=name;
                    new_printer->status=PRINTER_DISABLED;
                    new_printer->type=find_type(file_type)->name;
                    printer_array[index]=new_printer;
                    free(args);
                    fprintf(out,"PRINTER:id:%i  name:%s  status:%s  type:%s\n",new_printer->id,new_printer->name,printer_status_names[index],new_printer->type);
                    sf_printer_defined(new_printer->name,new_printer->type);
                    sf_printer_status(new_printer->name,new_printer->status);
                    sf_cmd_ok();
                    return 1;
                }else{
                    free(args);
                    fprintf(out,"Printer name is already used.\n");
                    sf_cmd_error("Name is already used.");
                    return 1;
                }
            }else{
                free(args);
                fprintf(out,"Printer still needs file_type argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
        }else{
            free(args);
            fprintf(out,"Printer needs name and file_type argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }else if(strcmp(args_token,"conversion")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char* file_type1=args_token;
            if(find_type(file_type1)==NULL){
                free(args);
                fprintf(out,"File type not found\n");
                sf_cmd_error("File type not found");
                return 1;
            }
            if((args_token=strtok(NULL," "))!=NULL){
                char* file_type2=args_token;
                if(find_type(file_type2)==NULL){
                    free(args);
                    fprintf(out,"File type not found\n");
                    sf_cmd_error("File type not found");
                    return 1;
                }
                if((args_token=strtok(NULL," "))!=NULL){
                    char** cmd_and_args=malloc(10*sizeof(char*));
                    int i=0;
                    cmd_and_args[i]=args_token;
                    i++;
                    while((args_token=strtok(NULL," "))!=NULL){
                        cmd_and_args[i]=args_token;
                        i++;
                    }
                    sf_conversion_defined(find_type(file_type1)->name,find_type(file_type2)->name,cmd_and_args);
                    free(args);
                    free(cmd_and_args);
                    sf_cmd_ok();
                    return 1;
                }else{
                    free(args);
                    fprintf(out,"Conversion still needs conversion_program argument\n");
                    sf_cmd_error("Invalid number of args");
                    return 1;
                }
            }else{
                free(args);
                fprintf(out,"Conversion still needs file_type2 and conversion_program arguments\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
        }else{
            free(args);
            fprintf(out,"Conversion still needs file_type1 file_type2 and conversion_program arguments\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }
    //Informational commands
    else if(strcmp(args_token,"printers")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            free(args);
            fprintf(out,"Printers doesn't require any additional arguments\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
        for(int i=0;i<MAX_PRINTERS;i++){
            if(printer_array[i]!=NULL){
                fprintf(out,"PRINTER:id=%i  name=%s  status=%s  type=%s\n",printer_array[i]->id,printer_array[i]->name,printer_status_names[i],printer_array[i]->type);
                sf_printer_status(printer_array[i]->name,printer_array[i]->status);
            }
        }
        free(args);
        sf_cmd_ok();
        return 1;
    }else if(strcmp(args_token,"jobs")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            free(args);
            fprintf(out,"Jobs doesn't require any additional arguments\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
        for(int i=0;i<MAX_JOBS;i++){
            if(job_array[i]!=NULL){
                fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",i,job_array[i]->type,job_status_names[i],job_array[i]->file);
                sf_job_status(i,job_array[i]->status);
            }
        }
        free(args);
        sf_cmd_ok();
        return 1;
    }
    //Spooling commands
    else if(strcmp(args_token,"print")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char* file_name=args_token;
            char* file_type=infer_file_type(file_name)->name;
            if(file_type==NULL){
                free(args);
                fprintf(out,"File type can't be inferred.\n");
                sf_cmd_error("File type infer error");
                return 1;
            }
            if(find_type(file_type)==NULL){
                free(args);
                fprintf(out,"File type not defined\n");
                sf_cmd_error("File type not found");
                return 1;
            }
            char* file=malloc(strlen(file_name)+1);
            memcpy(file,file_name,strlen(file_name)+1);
            JOB *job=malloc(sizeof(JOB));
            job->type=file_type;
            job->status=JOB_CREATED;
            job->file=file;
            int file_arg_counter=0;
            while((args_token=strtok(NULL," "))!=NULL){
                if(find_printer_index(args_token)!=-1){
                    job->eligible_printers[file_arg_counter]=printer_array[find_printer_index(args_token)];
                }else{
                    file_arg_counter=-1;
                    break;
                }
            }
            if(file_arg_counter==-1){
                free(job->file);
                free(job);
                free(args);
                fprintf(out,"Can't find one or more of the printer(s) name given\n");
                sf_cmd_error("Printer(s) name given is not found");
                return 1;
            }
            job->eligible=file_arg_counter;
            int index=new_job_index(job);
            job_array[index]=job;
            update_job_status(job->status,index);
            free(args);
            sf_job_created(index,job->file,job->type);
            if(job->eligible==0){
                for(int i=0;i<MAX_PRINTERS;i++){
                    if(printer_array[i]!=NULL){
                        if(printer_array[i]->status==PRINTER_IDLE){    

                        }
                    }
                }
                fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",index,job->type,job_status_names[index],job->file);
                sf_cmd_ok();
                return 1;
            }else{
                for(int i=0;i<job->eligible;i++){
                    if(job->eligible_printers[i]->status==PRINTER_IDLE){
                               
                    }
                }
                fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",index,job->type,job_status_names[index],job->file);
                sf_cmd_ok();
                return 1;
            }
        }else{
            fprintf(out,"Missing file name argument\n");
            sf_cmd_error("Invalid number of args");
            free(args);
            return 1;
        }
    }else if(strcmp(args_token,"cancel")==0){
        if((args_token=strtok(NULL," "))!=NULL){
                    
        }
    }else if(strcmp(args_token,"pause")==0){
        if((args_token=strtok(NULL," "))!=NULL){
                    
        }
    }else if(strcmp(args_token,"resume")==0){
        if((args_token=strtok(NULL," "))!=NULL){
                    
        }
    }else if(strcmp(args_token,"disable")==0){
        if((args_token=strtok(NULL," "))!=NULL){
                    
        }
    }else if(strcmp(args_token,"enable")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char* printer_name=args_token;
            if((args_token=strtok(NULL," "))!=NULL){
                free(args);
                fprintf(out,"Enable only needs printer name argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            int index=find_printer_index(printer_name);
            if(index!=-1){
                PRINTER *printer=printer_array[index];
                printer->status=PRINTER_IDLE;
                update_printer_status(PRINTER_IDLE,printer->name);
                for(int i=0;i<MAX_JOBS;i++){
                    if(job_array[i]!=NULL){
                        if(is_eligible_printer(job_array[i],printer)==0){
                            int pfd=imp_connect_to_printer(printer->name,printer->type,PRINTER_NORMAL);
                            int jfd=open(job_array[i]->file,O_RDONLY);
                            pid_t pid;
                            char *arg[]={"cat","/bin/cat",NULL};
                            if(pfd<0 || jfd<0){
                                perror("Can't open file");
                                return 1;
                            }
                            char* from=job_array[i]->type;
                            char* to=printer->type;
                            if((pid=fork())<0){
                                perror("Can't fork");
                                return 1;
                            }else{
                                if(pid==0){
                                    CONVERSION **conversion=find_conversion_path(from,to);
                                    if(conversion[0]==NULL && strcmp(from,to)==0){
                                        int val=execvp(arg[0],arg);
                                        if(val==-1){
                                            perror("Can't execute this. Something went wrong");
                                        }
                                    }
                                    if(conversion[0] && strcmp(from,to)){
                                        perror("There is no conversion path between these two files");
                                        free(args);
                                        sf_cmd_error("Can't find conversion path");
                                        
                                    }
                                    char **cmds=malloc(sizeof(conversion));
                                    for(int i=0;i<sizeof(cmds);i++){
                                        if(define_conversion(conversion[i]->from,conversion[i]->to,conversion[i]->cmd_and_args)!=NULL){
                                            
                                        }
                                        cmds[i]=conversion[i]->cmd_and_args[0];
                                    }
                                    sf_job_started(i,printer->name,getpid(),cmds);
                                    int pgid=setpgid(0,0);
                                    if(pgid<0){
                                        perror("Can't create process group");
                                    }else{
                                        pgid=fork();
                                        if(pgid<0){
                                            perror("Couldn't fork process");
                                        }else{
                                            if(pgid==0){
                                                
                                            }
                                        }
                                    }
                                }

                            }
                        }
                        continue;
                    }
                }
                free(args);
                sf_cmd_ok();
                return 1;
            }else{
                free(args);
                fprintf(out,"Printer not found");
                sf_cmd_error("Invalid printer arg");
                return 1;
            }
        }else{
            free(args);
            fprintf(out,"Missing printer name argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }else{
        free(args);
        fprintf(out,"Unrecognized command\n");
        sf_cmd_error("Invalid command");
    }
    return 1;
}

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

