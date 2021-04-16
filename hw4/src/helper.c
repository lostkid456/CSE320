#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//For forking and signals
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "imprimer.h"
#include "sf_readline.h"
#include "conversions.h"
#include "helper.h"

int parse_inp(FILE *in,FILE *out,char* args){
    job_flag=0;
    if(args==NULL){
        return 0;
    }
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
        int fl=0;
        while(waitpid(-1,&fl,WNOHANG)>0){}
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
                    char** cmd_and_args=malloc(100*sizeof(char*));
                    int i=0;
                    cmd_and_args[i]=args_token;
                    i++;
                    while((args_token=strtok(NULL," "))!=NULL){
                        cmd_and_args[i]=args_token;
                        i++;
                    }
                    define_conversion(find_type(file_type1)->name,find_type(file_type2)->name,cmd_and_args);
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
            if(infer_file_type(file_name)==NULL){
                free(args);
                fprintf(out,"File type can't be inferred.\n");
                sf_cmd_error("File type infer error");
                return 1;
            }
            char *file_type=infer_file_type(file_name)->name;
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
            job->id=index;
            job_array[index]=job;
            update_job_status(job->status,index);
            free(args);
            sf_job_created(index,job->file,job->type);
            int counter=0;
            if(job->eligible==0){
                for(int i=0;i<MAX_PRINTERS;i++){
                    if(printer_array[i]!=NULL){
                        if(printer_array[i]->status==PRINTER_IDLE){
                            counter+=1;  
                            conversion_pipeline(printer_array[i],job);
                        }
                    }
                }
                for(int i=0;i<MAX_JOBS;i++){
                    if(i==job->id){
                        continue;
                    }
                    for(int j=0;j<MAX_PRINTERS;j++){
                        if(printer_array[j]!=NULL){
                            if(job_array[i]==NULL && printer_array[j]->status==PRINTER_IDLE){
                                counter+=1;
                                conversion_pipeline(printer_array[j],job_array[i]);
                            }
                        }
                    }
                }
            }else{
                for(int i=0;i<job->eligible;i++){
                    if(job->eligible_printers[i]->status==PRINTER_IDLE){
                        counter+=1;
                        conversion_pipeline(job->eligible_printers[i],job);
                    }
                }
                for(int i=0;i<MAX_JOBS;i++){
                    if(i==job->id){
                        continue;
                    }
                    for(int j=0;j<MAX_PRINTERS;j++){
                        if(printer_array[j]!=NULL){
                            if(job_array[i]==NULL && printer_array[j]->status==PRINTER_IDLE){
                                counter+=1;
                                conversion_pipeline(printer_array[j],job_array[i]);
                            }
                        }
                    }
                }
            }
            if(counter==0){
                fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",index,job->type,job_status_names[index],job->file);
            }
            sf_cmd_ok();
            return 1;
        }else{
            fprintf(out,"Missing file name argument\n");
            sf_cmd_error("Invalid number of args");
            free(args);
            return 1;
        }
    }else if(strcmp(args_token,"cancel")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            int job_number=atoi(args_token);
            if((args_token=strtok(NULL," "))!=NULL){
                free(args);
                fprintf(out,"Cancel only requires a job number argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            if(job_number>=MAX_JOBS || job_number<0){
                free(args);
                fprintf(out,"Cancel only requires a job number argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            if(job_array[job_number]->status==JOB_PAUSED){
                killpg(job_array[job_number]->pid,SIGCONT);
            }
            if(job_array[job_number]->status==JOB_RUNNING){
                killpg(job_array[job_number]->pid,SIGTERM);
                sf_job_status(job_array[job_number]->id,JOB_ABORTED);
                update_job_status(JOB_ABORTED,job_number);
                job_array[job_number]->status=JOB_ABORTED;
                free(args);
                sf_cmd_ok();
            }
            else{
                fprintf(out,"This job is not running. No need to cancel");
                free(args);
                sf_cmd_ok();
            }
        }else{
            free(args);
            fprintf(out,"Cancel requires a job number argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }else if(strcmp(args_token,"pause")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            int job_number=atoi(args_token);
            if((args_token=strtok(NULL," "))!=NULL){
                free(args);
                fprintf(out,"Pause only requires a job number argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            if(job_array[job_number]->status==JOB_RUNNING){
                killpg(job_array[job_number]->pid,SIGSTOP); 
                sf_job_status(job_array[job_number]->id,JOB_PAUSED);
                update_job_status(JOB_PAUSED,job_number);
                free(args);
                sf_cmd_ok();
            }else{
                fprintf(out,"This job is not running. No need to pause");
                free(args);
                sf_cmd_ok();
            } 
        }else{
            free(args);
            fprintf(out,"Pause requires a job number argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }else if(strcmp(args_token,"resume")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            int job_number=atoi(args_token);
            if((args_token=strtok(NULL," "))!=NULL){
                free(args);
                fprintf(out,"Resume only requires a job number argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            if(job_array[job_number]->status==JOB_PAUSED){
                killpg(job_array[job_number]->pid,SIGCONT); 
                sf_job_status(job_array[job_number]->id,JOB_RUNNING);
                update_job_status(JOB_RUNNING,job_number);
                free(args);
                sf_cmd_ok();
            }else{
                fprintf(out,"This job is not stopped. No need to resume");
                free(args);
                sf_cmd_error("operation can't be done");
            } 
        }else{
            free(args);
            fprintf(out,"Resume requires a job number argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
        }
    }else if(strcmp(args_token,"disable")==0){
        if((args_token=strtok(NULL," "))!=NULL){
            char* printer_name=args_token;
            if((args_token=strtok(NULL," "))!=NULL){
                free(args);
                fprintf(out,"Disable only needs printer name argument\n");
                sf_cmd_error("Invalid number of args");
                return 1;
            }
            int index=find_printer_index(printer_name); 
            if(index!=-1){
                sf_printer_status(printer_array[index]->name,PRINTER_DISABLED);
                update_printer_status(PRINTER_DISABLED,printer_array[index]->name);
                printer_array[index]->status=PRINTER_DISABLED;
                free(args);
                sf_cmd_ok();
            }else{
                free(args);
                fprintf(out,"Printer cannot be found");
                sf_cmd_error("Printer not found");
                return 1;
            } 
        }else{
            free(args);
            fprintf(out,"Disable requires a printer name argument\n");
            sf_cmd_error("Invalid number of args");
            return 1;
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
                            conversion_pipeline(printer,job_array[i]);
                        }
                    }
                }
                free(args);
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
        printer_status_names[find_printer_index(name)]="disabled";
    }else if(status==PRINTER_IDLE){
        printer_status_names[find_printer_index(name)]="idle";
    }else{
        printer_status_names[find_printer_index(name)]="busy";
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
        job_status_names[i]=NULL;
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

int printer_from_pid(pid_t pid){
    for(int i=0;i<MAX_PRINTERS;i++){
        if(printer_array[i]!=NULL){
            if(printer_array[i]->pid==pid){
                return i;
            }
        }
    }
    return -1;
}

int job_from_pid(pid_t pid){
    for(int i=0;i<MAX_JOBS;i++){
        if(job_array[i]!=NULL){
            if(job_array[i]->pid==pid){
                return i;
            }   
        }
    }
    return -1;
}

//Callback function
void sig_handler_parent(){
    int flag;
    if(job_flag==1){
        pid_t return_val;
        while((return_val=waitpid(-1,&flag,WNOHANG))>0){
            int printer_index=printer_from_pid(return_val);
            int job_index=job_from_pid(return_val);
            if(WIFEXITED(flag)){
                if(WEXITSTATUS(flag)==0){
                    if(printer_index!=-1 && job_index!=-1){
                        job_array[job_index]->status=JOB_FINISHED;
                        printer_array[printer_index]->status=PRINTER_IDLE;
                        update_job_status(JOB_FINISHED,job_index);
                        update_printer_status(PRINTER_IDLE,printer_array[printer_index]->name);
                        sf_job_finished(job_array[job_index]->id,JOB_FINISHED);
                        sf_job_status(job_array[job_index]->id,JOB_FINISHED);
                        sf_printer_status(printer_array[printer_index]->name,PRINTER_IDLE);
                        clock_t now=clock();
                        while(1){
                            if(clock()-now>=10){
                                break;
                            }
                        }
                        sf_job_deleted(job_array[job_index]->id);
                        sf_job_status(job_array[job_index]->id,JOB_DELETED);
                        update_job_status(JOB_DELETED,job_index);
                        job_array[job_index]=NULL;
                    }
                }
                }else{
                    if(printer_index!=-1 && job_index!=-1){
                        job_array[job_index]->status=JOB_ABORTED;
                        printer_array[printer_index]->status=PRINTER_IDLE;
                        update_job_status(JOB_ABORTED,job_index);
                        update_printer_status(PRINTER_IDLE,printer_array[printer_index]->name);
                        sf_job_aborted(job_array[job_index]->id,JOB_ABORTED);
                        sf_job_status(job_array[job_index]->id,JOB_ABORTED);
                        sf_printer_status(printer_array[printer_index]->name,PRINTER_IDLE);
                        clock_t now=clock();
                        while(1){
                            if(clock()-now>=10){
                                break;
                            }
                        }
                        sf_job_deleted(job_array[job_index]->id);
                        sf_job_status(job_array[job_index]->id,JOB_DELETED);
                        update_job_status(JOB_DELETED,job_index);
                        job_array[job_index]=NULL;
                    }
                }
            if(WIFSTOPPED(flag)){
                if(job_index!=-1){
                    job_array[job_index]->status=JOB_PAUSED;
                    printer_array[printer_index]->status=PRINTER_IDLE;
                    update_job_status(JOB_PAUSED,job_index);
                    update_printer_status(PRINTER_IDLE,printer_array[printer_index]->name);
                    sf_job_status(job_array[job_index]->id,JOB_ABORTED);
                    sf_printer_status(printer_array[printer_index]->name,PRINTER_IDLE);
                    sf_cmd_ok();
                }
            }
            if(WIFCONTINUED(flag)){
                if(job_index!=-1){
                    job_array[job_index]->status=JOB_RUNNING;
                    printer_array[printer_index]->status=PRINTER_BUSY;
                    update_job_status(JOB_RUNNING,job_index);
                    update_printer_status(PRINTER_BUSY,printer_array[printer_index]->name);
                    sf_job_status(job_array[job_index]->id,JOB_RUNNING);
                    sf_printer_status(printer_array[printer_index]->name,PRINTER_BUSY);
                    sf_cmd_ok();
                }
            }
            if(WIFSIGNALED(flag)){
                if(printer_index!=-1 && job_index!=-1){
                    job_array[job_index]->status=JOB_ABORTED;
                    printer_array[printer_index]->status=PRINTER_IDLE;
                    update_job_status(JOB_ABORTED,job_index);
                    update_printer_status(PRINTER_IDLE,printer_array[printer_index]->name);
                    sf_job_aborted(job_array[job_index]->id,JOB_ABORTED);
                    sf_job_status(job_array[job_index]->id,JOB_ABORTED);
                    sf_printer_status(printer_array[printer_index]->name,PRINTER_IDLE);
                    clock_t now=clock();
                    while(1){
                        if(clock()-now>=10){
                            break;
                        }
                    }
                    sf_job_deleted(job_array[job_index]->id);
                    sf_job_status(job_array[job_index]->id,JOB_DELETED);
                    update_job_status(JOB_DELETED,job_index);
                    job_array[job_index]=NULL;
                }
            }
        }
    }
}

void sig_handler_child(int signal){
    job_flag=1;
}

int conversion_pipeline(PRINTER *printer,JOB* job){ 
        pid_t pid;
        char* from=job->type;
        char* to=printer->type;
        CONVERSION **conversion=find_conversion_path(from,to);
        if(conversion==NULL && strcmp(from,to)){
            perror("No path");
            return -1;
        } 
        int conversion_length=0;
        for(int i=0;conversion[i]!=NULL;i++){
            conversion_length++;
        }
        char **cmds=malloc(sizeof(char*)*(conversion_length+1));
        for(int i=0;i<conversion_length;i++){
            cmds[i]=conversion[i]->cmd_and_args[0];
        }
        cmds[conversion_length]=NULL;
        //Job started
        sf_job_started(job->id,printer->name,getpid(),cmds);
        free(cmds);
        //Update job and printer status
        update_job_status(JOB_RUNNING,job->id);
        job->status=JOB_RUNNING;
        update_printer_status(PRINTER_BUSY,printer->name);
        printer->status=PRINTER_BUSY;
        sf_job_status(job->id,job->status);
        sf_printer_status(printer->name,printer->status);
        sf_cmd_ok();
        pid=fork();
        if(pid<0){
            perror("Couldn't fork");
            return -1;
        }
        if(setpgid(0,0)<0){
            perror("Couldn't process group");
        }
        if(pid==0){
            if(strcmp(from,to)==0){
                int jfd=open(job->file,O_RDONLY);
                int pfd=imp_connect_to_printer(printer->name,printer->type,PRINTER_NORMAL);
                dup2(jfd,STDIN_FILENO);
                dup2(pfd,STDOUT_FILENO);
                close(jfd);
                close(pfd);
                char *arg[]={"cat",NULL};
                if(execvp(arg[0],arg)==-1){
                    perror("Couldn't execute");
                    return -1;
                }
            }
            int pipefd[2*(conversion_length)];
            for(int i=0;conversion[i]!=NULL;i++){
                if(pipe(pipefd+(2*i))<0){
                    perror("Couldn't pipe");
                    return -1;
                }
            }
            for(int i=0;conversion[i]!=NULL;i++){
                if((pid=fork())<0){
                    perror("Couldn't fork");
                    return -1;
                }else{
                    if(pid==0){
                        if(i==0){
                            int jfd=open(job->file,O_RDONLY);
                            if(jfd<0){
                                exit(EXIT_FAILURE);
                            }
                            //Input from file(job_file)
                            if(dup2(jfd,STDIN_FILENO)<0){
                                perror("Couldn't dup2");
                                exit(EXIT_FAILURE);
                            }
                        }else{
                            if(dup2(pipefd[2*i],STDIN_FILENO)<0){
                                perror("Couldn't dup2");
                                exit(EXIT_FAILURE);
                            }
                        }
                        if(conversion[i+1]==NULL){
                            int pfd=imp_connect_to_printer(printer->name,printer->type,PRINTER_NORMAL);
                            //Output from file(printer_file)
                            if(dup2(pfd,STDOUT_FILENO)<0){
                                perror("Couldn't dup2");
                                exit(EXIT_FAILURE);
                            }
                        }else{
                            if(dup2(pipefd[(2*i)+1],STDOUT_FILENO)<0){
                                    perror("Couldn't dup2");
                                    exit(EXIT_FAILURE);
                            }
                        }
                        for(int i=0;i<conversion_length+1;i++){
                            close(pipefd[i*2]);
                            close(pipefd[(2*i)+1]);
                        }
                        if(execvp(conversion[i]->cmd_and_args[0],conversion[i]->cmd_and_args)<0){
                                perror("Couldn't execute");
                                exit(EXIT_FAILURE);
                        }
                    }
                }
            }
            for(int i=0;conversion[i]!=NULL;i++){
                int flag=0;
                pid_t val=waitpid(-1,&flag,WNOHANG);
                if(val==-1){
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_SUCCESS);
        }else{
            job->pid=pid;
            printer->pid=pid;
        }
    return 0;
}

