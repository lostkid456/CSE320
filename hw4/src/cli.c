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
                    printf("Help does not require any additional arguments\n");
                    sf_cmd_error("Too many args");
                    free(args);
                    continue;
                }
                printf("Commands are: help, quit, type, printer, conversion, printers, jobs, print, cancel, disable, enable, pause and resume\n");
                sf_cmd_ok();
                free(args);
            }
            else if(strcmp(args_token,"quit")==0){
                if(strtok(NULL," ")!=NULL){
                    printf("Quit does not require any additional arguments\n");
                    sf_cmd_error("Too many args");
                    free(args);
                    continue;
                }
                free(args);
                for(int i=0;i<MAX_PRINTERS;i++){
                    if(printer_array[i]!=NULL){
                        free(printer_array[i]->name);
                        free(printer_array[i]);
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
                        printf("Type only requires one additional argument\n");
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
                            printf("Printer only requires two additional argument\n");
                            sf_cmd_error("Too many args");
                            free(args);
                            continue;
                        }
                        if(find_type(file_type)==NULL){
                            printf("Type is not defined\n");
                            sf_cmd_error("Can't find type");
                            free(args);
                            continue;
                        }
                        int index=get_printer_index(printer_name);
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
                            printf("PRINTER:id:%i  name:%s  status:%s  type:%s\n",new_printer->id,new_printer->name,printer_status_names[index],new_printer->type);
                            sf_printer_defined(new_printer->name,new_printer->type);
                            sf_printer_status(new_printer->name,new_printer->status);
                            sf_cmd_ok();
                            free(args);
                            continue;
                        }else{
                            printf("Printer name is already used.\n");
                            sf_cmd_error("Name is already used.");
                            free(args);
                            continue;
                        }
                    }else{
                        printf("Printer still needs file_type argument\n");
                        sf_cmd_error("Invalid number of args");
                        free(args);
                        continue;
                    }
                }else{
                    printf("Printer needs name and file_type argument\n");
                    sf_cmd_error("Invalid number of args");
                    free(args);
                    continue;
                }
            }else if(strcmp(args_token,"conversion")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    char* file_type1=args_token;
                    if(find_type(file_type1)==NULL){
                        printf("File type not found\n");
                        sf_cmd_error("File type not found");
                        free(args);
                        continue;
                    }
                    if((args_token=strtok(NULL," "))!=NULL){
                        char* file_type2=args_token;
                        if(find_type(file_type2)==NULL){
                            printf("File type not found\n");
                            sf_cmd_error("File type not found");
                            free(args);
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
                            sf_cmd_ok();
                            free(args);
                            free(cmd_and_args);
                            continue;
                        }else{
                            printf("Conversion still needs conversion_program argument\n");
                            sf_cmd_error("Invalid number of args");
                            free(args);
                            continue;
                        }
                    }else{
                        printf("Conversion still needs file_type2 and conversion_program arguments\n");
                        sf_cmd_error("Invalid number of args");
                        free(args);
                        continue;
                    }
                }else{
                    printf("Conversion still needs file_type1 file_type2 and conversion_program arguments\n");
                    sf_cmd_error("Invalid number of args");
                    free(args);
                    continue;
                }
            }
            //Informational commands
            else if(strcmp(args_token,"printers")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    printf("Printers doesn't require any additional arguments\n");
                    sf_cmd_error("Invalid number of args");
                    free(args);
                    continue;
                }
                for(int i=0;i<MAX_PRINTERS;i++){
                    if(printer_array[i]!=NULL){
                        printf("PRINTER:id:%i  name:%s  status:%s  type:%s\n",printer_array[i]->id,printer_array[i]->name,printer_status_names[get_printer_index(printer_array[i]->name)],printer_array[i]->type);
                    }
                }
                sf_cmd_ok();
                free(args);
                continue;
            }else if(strcmp(args_token,"jobs")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    printf("Jobs doesn't require any additional arguments\n");
                    sf_cmd_error("Invalid number of args");
                    free(args);
                    continue;
                }
                for(int i=0;i<MAX_JOBS;i++){
                    if(job_array[i]!=NULL){

                    }
                }
                sf_cmd_ok();
                free(args);
                continue;
            }
            //Spooling commands
            else if(strcmp(args_token,"print")==0){
                if((args_token=strtok(NULL," "))!=NULL){
                    
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
                    
                }
            }else{
                free(args);
                printf("Unrecognized command\n");
                sf_cmd_error("Invalid command");
            }
        }
    }
    return 0;
}
