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

void *chla_client_service(void *arg){
    int connfd=*((int*)(arg));
    free(arg);
    CLIENT *new_client;
    pthread_t mailbox_tid;
    if((new_client=creg_register(client_registry,connfd))!=NULL){
        while(1){
            CHLA_PACKET_HEADER *new_header=malloc(sizeof(CHLA_PACKET_HEADER));
            char **args=malloc(sizeof(char *));
            if(proto_recv_packet(connfd,new_header,(void **)args)<0){
                break;
            }
            if((new_header->type)==CHLA_LOGIN_PKT){
                if(client_login(new_client,strtok(*args,"\r\n"))<0){
                    client_send_nack(new_client,new_header->msgid);
                }else{
                    client_send_ack(new_client,new_header->msgid,NULL,0);
                    //NEED TO CREATE MAILBOX THREAD
                    int *mailbox_fd=malloc(sizeof(int));
                    *mailbox_fd=client_get_fd(new_client);
                    pthread_create(&mailbox_tid,NULL,chla_mailbox_service,mailbox_fd);
                }
            }else if((new_header->type)==CHLA_LOGOUT_PKT){
                if(client_logout(new_client)<0){
                    client_send_nack(new_client,new_header->msgid);
                }else{
                    client_send_ack(new_client,new_header->msgid,NULL,0);
                }
            }else if((new_header->type)==CHLA_USERS_PKT){
                CLIENT **clients=creg_all_clients(client_registry);
                char* logged_user;
                for(int i=0;i<MAX_CLIENTS;i++){
                    if(clients[i]==NULL){
                        break;
                    }
                    client_unref(clients[i],"Dereference after use from creg_all for USERS");
                    USER *user;
                    if((user=client_get_user(clients[i],0))!=NULL){
                        if(i==0){
                            logged_user=malloc(strlen(user_get_handle(user))+2);
                            strcat(logged_user,user_get_handle(user));
                            strcat(logged_user,"\n");
                        }else{
                            logged_user=realloc(logged_user,sizeof(logged_user)+strlen(user_get_handle(user))+2);
                            strcat(logged_user,user_get_handle(user));
                            strcat(logged_user,"\n");
                        }
                    }
                }
                free(clients);
                if(logged_user==NULL){
                    client_send_ack(new_client,ntohl(new_header->msgid),NULL,0);
                }else{
                    client_send_ack(new_client,ntohl(new_header->msgid),logged_user,strlen(logged_user)+1);
                }
            }else if((new_header->type)==CHLA_SEND_PKT){
                
            }else{
                client_send_nack(new_client,new_header->msgid);
            }
            free(new_header);
        }
        client_logout(new_client);
        creg_unregister(client_registry,new_client);
    }
    pthread_exit(NULL);
    return NULL;
}

static void *chla_mailbox_service(void *arg){
    int mailbox_fd=*((int*)(arg));
    pthread_detach(pthread_self());
    free(arg);
    
    return NULL;
}