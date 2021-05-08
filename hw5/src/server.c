#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "client_registry.h"
#include "globals.h"
#include "server.h"
#include "client.h"

static void *chla_mailbox_service(void *arg);

void *chla_client_service(void *arg){
    int connfd=*((int*)(arg));
    free(arg);
    CLIENT *new_client;
    CHLA_PACKET_HEADER *new_header;
    char **args=calloc(1,sizeof(char *));
    pthread_t mailbox_tid;
    if((new_client=creg_register(client_registry,connfd))!=NULL){
        while(1){
            new_header=calloc(1,sizeof(CHLA_PACKET_HEADER));
            if(proto_recv_packet(connfd,new_header,(void **)args)<0){
                break;
            }
            char *temp_args_pointer=*args;
            //debug("PAYLOAD: %s",*args);
            if((new_header->type)==CHLA_LOGIN_PKT){
                if(client_login(new_client,strtok(*args,"\r\n"))<0){
                    client_send_nack(new_client,new_header->msgid);
                }else{
                    client_send_ack(new_client,new_header->msgid,NULL,0);
                    //NEED TO CREATE MAILBOX THREAD
                    pthread_create(&mailbox_tid,NULL,chla_mailbox_service,new_client);
                    pthread_detach(mailbox_tid);
                }
            }else if((new_header->type)==CHLA_LOGOUT_PKT){
                if(client_logout(new_client)<0){
                    client_send_nack(new_client,new_header->msgid);
                }else{
                    client_send_ack(new_client,new_header->msgid,NULL,0);
                }
            }else if((new_header->type)==CHLA_USERS_PKT){
                CLIENT **clients=creg_all_clients(client_registry);
                char* logged_user=calloc(1,sizeof(char*));
                char* user_handle=calloc(1,sizeof(char*));
                int counter=0;
                for(int i=0;i<MAX_CLIENTS;i++){
                    if(clients[i]==NULL){
                        break;
                    }
                    client_unref(clients[i],"Dereference after use from creg_all for USERS");
                    USER *user;
                    if((user=client_get_user(clients[i],0))!=NULL){
                        user_handle=realloc(user_handle,strlen(user_get_handle(user))+1);
                        memcpy(user_handle,user_get_handle(user),strlen(user_get_handle(user))+1);
                        logged_user=realloc(logged_user,(strlen(logged_user)+strlen(user_handle)+3));
                        strcat(logged_user,user_handle);
                        strcat(logged_user,"\r\n");
                        strcat(logged_user,"\0");
                        counter+=1;
                    }
                }
                free(clients);
                if(counter==0){
                    client_send_ack(new_client,ntohl(new_header->msgid),NULL,0);
                }else{
                    client_send_ack(new_client,ntohl(new_header->msgid),logged_user,strlen(logged_user)+1);
                    free(logged_user);
                    free(user_handle);
                }
            }else if((new_header->type)==CHLA_SEND_PKT){
                strcat(*args,"\0");
                CLIENT **clients=creg_all_clients(client_registry);
                char *temp_handle=strtok(*args,"\r\n");
                char *handle=calloc(1,strlen(temp_handle)+1);
                char* message;
                char *whole_thing;
                memcpy(handle,temp_handle,strlen(temp_handle));
                strcat(handle,"\0");
                int counter=0;
                for(int i=0;i<MAX_CLIENTS;i++){
                    if(clients[i]==NULL){
                        break;
                    }
                    client_unref(clients[i],"Dereference after use from creg_all for USERS");
                    USER *user;
                    if((user=client_get_user(clients[i],0))!=NULL){
                        if(strcmp(user_get_handle(user),handle)==0){
                            char *temp_message=strtok(NULL,"\r\n");
                            message=calloc(1,strlen(temp_message)+1);
                            memcpy(message,temp_message,strlen(temp_message));
                            strcat(message,"\0");
                            //debug("PAYLOAD: %s",message);
                            whole_thing=calloc(1,(strlen(handle)+strlen(message)+3));
                            strcat(whole_thing,handle);
                            strcat(whole_thing,"\r\n");
                            strcat(whole_thing,message);
                            strcat(whole_thing,"\0");
                            mb_add_message(client_get_mailbox(clients[i],0),ntohl(new_header->msgid),client_get_mailbox(new_client,1),(void*)(whole_thing),strlen(whole_thing)+1);
                            mb_unref(client_get_mailbox(clients[i],1),"now that message has been added to ");
                            client_send_ack(new_client,ntohl(new_header->msgid),NULL,0);
                            counter+=1;
                            free(message);
                        }
                    }
                }
                if(counter==0){
                    client_send_nack(new_client,new_header->msgid);
                }
                free(handle);
                free(clients);
            }else{
                client_send_nack(new_client,new_header->msgid);
            }
            free(temp_args_pointer);
            free(new_header);
        }
        free(new_header);
        free(args);
        client_logout(new_client);
        creg_unregister(client_registry,new_client);
        client_unref(new_client,"NO NEED FOR CLIENT");
    }
    return NULL;
}

static void *chla_mailbox_service(void *arg){
    CLIENT *client=(CLIENT *)(arg);
    MAILBOX *mailbox=client_get_mailbox(client,0);
    while(1){
        MAILBOX_ENTRY *entry;
        if((entry=mb_next_entry(mailbox))==NULL){
           break;
        }
        if(entry->type==MESSAGE_ENTRY_TYPE){
            CHLA_PACKET_HEADER *new_header=calloc(1,sizeof(CHLA_PACKET_HEADER));
            new_header->msgid=entry->content.message.msgid;
            new_header->payload_length=htonl(entry->content.message.length);
            new_header->type=CHLA_MESG_PKT;
            // if(client_send_packet(client,new_header,entry->content.message.body)<0){
            //     mb_add_notice(mailbox,BOUNCE_NOTICE_TYPE,entry->content.notice.msgid);
            // }else{
            //     mb_add_notice(mailbox,RRCPT_NOTICE_TYPE,entry->content.notice.msgid);
            // }
            client_send_packet(client,new_header,entry->content.message.body);
            mb_add_notice(mailbox,RRCPT_NOTICE_TYPE,entry->content.notice.msgid);
            free(new_header);
            free(entry->content.message.body);
            free(entry);
            continue;
        }
        if(entry->type==NOTICE_ENTRY_TYPE){
            CHLA_PACKET_HEADER *new_header=calloc(1,sizeof(CHLA_PACKET_HEADER));
            new_header->msgid=entry->content.message.msgid;
            new_header->payload_length=htonl(entry->content.message.length);
            // if(entry->content.notice.type==BOUNCE_NOTICE_TYPE){
            //     new_header->type=CHLA_BOUNCE_PKT;
            // }else{
            //     new_header->type=CHLA_RCVD_PKT;
            // }
            client_send_packet(client,new_header,entry->content.message.body);
            free(new_header);
            free(entry);
            continue;
        }
    }
    mb_unref(mailbox,"Ending it");
    return NULL;
}