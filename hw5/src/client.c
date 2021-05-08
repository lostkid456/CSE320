#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "user.h"
#include "client_registry.h"
#include "globals.h"
#include "client.h"

typedef struct client{
    int fd;
    volatile int ref_count;
    volatile int log_in;
    USER *user;
    MAILBOX *mailbox;
    sem_t mutex;
}CLIENT;

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd){
    CLIENT *new_client=calloc(1,sizeof(CLIENT));
    if(new_client==NULL){
        return NULL;
    }
    new_client->fd=fd;
    new_client->ref_count=1;
    new_client->log_in=0;
    sem_init(&new_client->mutex,0,1);
    return new_client;
}

CLIENT *client_ref(CLIENT *client, char *why){
    sem_wait(&client->mutex);
    debug("Referencing client %s %d",why,client->ref_count);
    (client->ref_count)+=1;
    sem_post(&client->mutex);
    return client;
}

void client_unref(CLIENT *client, char *why){
    sem_wait(&client->mutex);
    debug("Unreferencing client %s %d",why,client->ref_count);
    (client->ref_count)-=1;
    if(client->ref_count==0){
        sem_post(&client->mutex);
        sem_destroy(&client->mutex);
        free(client);
        debug("Freeing Client");
        return;
    }
   sem_post(&client->mutex);
}

int client_login(CLIENT *client, char *handle){
    if(client->log_in==1){
        return -1;
    }
    static sem_t login_lock;
    sem_init(&login_lock,0,1);
    CLIENT **all_clients=creg_all_clients(client_registry);
    sem_wait(&login_lock);
    for(int i=0;i<MAX_CLIENTS;i++){
        client_unref(all_clients[i],"Dereference for login");
        debug("Derefernce for creg_all_clients");
        if((all_clients[i]->user)!=NULL){
            if(strcmp(user_get_handle(all_clients[i]->user),handle)==0){
                debug("ALREADY LOGGED IN %s",user_get_handle(all_clients[i]->user));
                free(all_clients);
                sem_post(&login_lock);
                return -1;
            }
        }else{
            break;
        }
    }
    free(all_clients);
    client->user=ureg_register(user_registry,handle);
    client->mailbox=mb_init(handle);
    client->log_in=1;
    debug("LOGGED IN %s",user_get_handle(client->user));
    sem_post(&login_lock);
    return 0;
}

int client_logout(CLIENT *client){
    sem_wait(&client->mutex);
    if(client->log_in!=1){
        sem_post(&client->mutex);
        return -1;
    }
    user_unref(client->user,"User reference decrease");
    client->user=NULL;
    mb_shutdown(client->mailbox);
    mb_unref(client->mailbox,"FINAL Unref mailbox");
    client->mailbox=NULL;
    client->log_in=0;
    debug("LOGGED OUT");
    sem_post(&client->mutex);
    return 0;
}

USER *client_get_user(CLIENT *client, int no_ref){
    if(client->user==NULL){
        return NULL;
    }
    sem_wait(&client->mutex);
    if(client->log_in!=1){
        sem_post(&client->mutex);
        return NULL;
    }
    if(no_ref!=0){
        sem_post(&client->mutex);
        return client->user;
    }else{
        sem_post(&client->mutex);
        return user_ref(client->user,"Increasing refernce");
    }
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref){
    if(client->mailbox==NULL){
        return NULL;
    }
    sem_wait(&client->mutex);
    if(client->log_in!=1){
        sem_post(&client->mutex);
        return NULL;
    }
    if(no_ref!=0){
        sem_post(&client->mutex);
        return client->mailbox;
    }else{
        sem_post(&client->mutex);
        debug("increasing reference of mailbox from client");
        mb_ref(client->mailbox,"Increasing reference");
        return client->mailbox;
    }
}

int client_get_fd(CLIENT *client){
    if(client==NULL){
        return 0;
    }
    int fd=client->fd;
    return fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data){
    sem_wait(&user->mutex);
    size_t pack_len=ntohl(pkt->payload_length);
    if(write(user->fd,pkt,sizeof(*pkt))==sizeof(*pkt)){
        if(data!=NULL){
            if(write(user->fd,data,pack_len)==pack_len){
                sem_post(&user->mutex);
                return 0;
            }else{
                sem_post(&user->mutex);
                return -1;
            }
        }
    }
    sem_post(&user->mutex);
    return -1;
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen){
    struct timeval curr_time;
    gettimeofday(&curr_time,NULL);
    CHLA_PACKET_HEADER *ack_pkt=calloc(1,sizeof(CHLA_PACKET_HEADER));
    ack_pkt->payload_length=ntohl(datalen);
    ack_pkt->msgid=msgid;
    ack_pkt->type=CHLA_ACK_PKT;
    ack_pkt->timestamp_sec=curr_time.tv_sec;
    ack_pkt->timestamp_nsec=curr_time.tv_usec*1000;
    if(client_send_packet(client,ack_pkt,data)<0){
        free(ack_pkt);
        return -1;
    }
    free(ack_pkt);
    return 0;
}

int client_send_nack(CLIENT *client, uint32_t msgid){
    struct timeval curr_time;
    gettimeofday(&curr_time,NULL);
    CHLA_PACKET_HEADER *nack_pkt=calloc(1,sizeof(CHLA_PACKET_HEADER));
    nack_pkt->type=CHLA_NACK_PKT;
    nack_pkt->msgid=msgid;
    nack_pkt->payload_length=0;
    nack_pkt->timestamp_sec=curr_time.tv_sec;
    nack_pkt->timestamp_nsec=curr_time.tv_usec*1000;
    if(client_send_packet(client,nack_pkt,NULL)<0){
        free(nack_pkt);
        return -1;
    }
    free(nack_pkt);
    return 0;
}