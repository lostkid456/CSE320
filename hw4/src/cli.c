/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//For forking
#include <unistd.h>
#include <sys/types.h>

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
        char *args;
        char *args_token;
        while(1){
            args=sf_readline("imp>");
            args_token=strtok(args," ");
            if(args_token==NULL){
                free(args);
                continue;
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
                    continue;
                }
                fprintf(out,"Commands are: help, quit, type, printer, conversion, printers, jobs, print, cancel, disable, enable, pause and resume\n");
                free(args);
                sf_cmd_ok();
            }
            else if(strcmp(args_token,"quit")==0){
                if(strtok(NULL," ")!=NULL){
                    free(args);
                    fprintf(out,"Quit does not require any additional arguments\n");
                    sf_cmd_error("Too many args");
                    continue;
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
                        continue;
                    }
                    define_type(file_type);
                    free(args);
                    sf_cmd_ok();
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
                            continue;
                        }
                        if(find_type(file_type)==NULL){
                            free(args);
                            fprintf(out,"Type is not defined\n");
                            sf_cmd_error("Can't find type");
                            continue;
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
                            continue;
                        }else{
                            free(args);
                            fprintf(out,"Printer name is already used.\n");
                            sf_cmd_error("Name is already used.");
                            continue;
                        }
                    }else{
                        free(args);
                        fprintf(out,"Printer still needs file_type argument\n");
                        sf_cmd_error("Invalid number of args");
                        continue;
                    }
                }else{
                    free(args);
                    fprintf(out,"Printer needs name and file_type argument\n");
                    sf_cmd_error("Invalid number of args");
                    continue;
                }
            }else if(strcmp(args_token,"conversion")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    char* file_type1=args_token;
                    if(find_type(file_type1)==NULL){
                        free(args);
                        fprintf(out,"File type not found\n");
                        sf_cmd_error("File type not found");
                        continue;
                    }
                    if((args_token=strtok(NULL," "))!=NULL){
                        char* file_type2=args_token;
                        if(find_type(file_type2)==NULL){
                            free(args);
                            fprintf(out,"File type not found\n");
                            sf_cmd_error("File type not found");
                            continue;
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
                            continue;
                        }else{
                            free(args);
                            fprintf(out,"Conversion still needs conversion_program argument\n");
                            sf_cmd_error("Invalid number of args");
                            continue;
                        }
                    }else{
                        free(args);
                        fprintf(out,"Conversion still needs file_type2 and conversion_program arguments\n");
                        sf_cmd_error("Invalid number of args");
                        continue;
                    }
                }else{
                    free(args);
                    fprintf(out,"Conversion still needs file_type1 file_type2 and conversion_program arguments\n");
                    sf_cmd_error("Invalid number of args");
                    continue;
                }
            }
            //Informational commands
            else if(strcmp(args_token,"printers")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    free(args);
                    fprintf(out,"Printers doesn't require any additional arguments\n");
                    sf_cmd_error("Invalid number of args");
                    continue;
                }
                for(int i=0;i<MAX_PRINTERS;i++){
                    if(printer_array[i]!=NULL){
                        fprintf(out,"PRINTER:id=%i  name=%s  status=%s  type=%s\n",printer_array[i]->id,printer_array[i]->name,printer_status_names[i],printer_array[i]->type);
                    }
                }
                free(args);
                sf_cmd_ok();
                continue;
            }else if(strcmp(args_token,"jobs")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    free(args);
                    fprintf(out,"Jobs doesn't require any additional arguments\n");
                    sf_cmd_error("Invalid number of args");
                    continue;
                }
                for(int i=0;i<MAX_JOBS;i++){
                    if(job_array[i]!=NULL){
                        fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",i,job_array[i]->type,job_status_names[i],job_array[i]->file);
                    }
                }
                free(args);
                sf_cmd_ok();
                continue;
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
                        continue;
                    }
                    if(find_type(file_type)==NULL){
                        free(args);
                        fprintf(out,"File type not defined\n");
                        sf_cmd_error("File type not found");
                        continue;
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
                        continue;
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
                        continue;
                    }else{
                        for(int i=0;i<job->eligible;i++){
                            if(job->eligible_printers[i]->status==PRINTER_IDLE){
                               
                            }
                        }
                        fprintf(out,"JOB[%i]: type=%s, status=%s, file=%s\n",index,job->type,job_status_names[index],job->file);
                        sf_cmd_ok();
                        continue;
                    }
                }else{
                    fprintf(out,"Missing file name argument\n");
                    sf_cmd_error("Invalid number of args");
                    free(args);
                    continue;
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
                        continue;
                    }
                    int index=find_printer_index(printer_name);
                    if(index!=-1){
                        PRINTER *printer=printer_array[index];
                        printer->status=PRINTER_IDLE;
                        update_printer_status(PRINTER_IDLE,printer->name);
                        for(int i=0;i<MAX_JOBS;i++){
                            if(job_array[i]!=NULL){
                                if(is_eligible_printer(job_array[i],printer)==0){
                                    int fd=imp_connect_to_printer(printer->name,printer->type,PRINTER_NORMAL);
                                    
                                }
                                continue;
                            }
                        }

                    }else{
                        free(args);
                        fprintf(out,"Printer not found");
                        sf_cmd_error("Invalid printer arg");
                        continue;
                    }
                }else{
                    free(args);
                    fprintf(out,"Missing printer name argument\n");
                    sf_cmd_error("Invalid number of args");
                    continue;
                }
            }else{
                free(args);
                fprintf(out,"Unrecognized command\n");
                sf_cmd_error("Invalid command");
            }
        }
    }
    if(in!=stdin && out!=stdout){
        
    }
    if(in!=stdin && out==stdout){
        
    }
    return 0;
}
