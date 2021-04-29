#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "debug.h"
#include "client_registry.h"
#include "client.h"


typedef struct client_registry{
    CLIENT **clients;
    volatile int total_clients;
    volatile int total_process;
    sem_t mutex;
    sem_t mutex2;
}CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *client_registry=malloc(sizeof(CLIENT_REGISTRY));
    if(client_registry==NULL){
        return NULL;
    }
    client_registry->clients=calloc(MAX_CLIENTS,sizeof(CLIENT*));
    client_registry->total_clients=0;
    client_registry->total_process=0;
    sem_init(&client_registry->mutex,0,1);
    sem_init(&client_registry->mutex2,0,1);
    debug("Initialize client registry");
    return client_registry;
}

void creg_fini(CLIENT_REGISTRY *cr){
    sem_wait(&cr->mutex);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]!=NULL){
            free(cr->clients[i]);
        }
    }
    sem_post(&cr->mutex);
    free(cr);
}

CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd){
    if(cr==NULL){
        return NULL;
    }
    sem_wait(&cr->mutex);
    if(cr->total_clients==MAX_CLIENTS){
        sem_post(&cr->mutex);
        return NULL;
    }
    CLIENT *new_client=client_create(cr,fd);
    if(new_client==NULL){
        sem_post(&cr->mutex);
        return NULL;
    }
    client_ref(new_client,"NEW REFERENCE");
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]==NULL){
            cr->clients[i]=new_client;
            (cr->total_clients)+=1;
            (cr->total_process)+=1;
        }
    }
    sem_post(&cr->mutex);
    return new_client;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client){
    sem_wait(&cr->mutex);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(client_get_fd(cr->clients[i])==client_get_fd(client)){
            client_unref(cr->clients[i],"Dereferencing");
            (cr->total_clients)-=1;
            if(cr->total_clients==0){
                
            }
            sem_post(&cr->mutex);
            return 0;
        }
    }
    sem_post(&cr->mutex);
    return -1;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr){
    sem_wait(&cr->mutex);
    CLIENT **client_list=calloc(MAX_CLIENTS,sizeof(CLIENT*));
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]!=NULL){
            client_list[i]=cr->clients[i];
        }
    }
    sem_post(&cr->mutex);
    return client_list;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    sem_wait(&cr->mutex2);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]!=NULL){
            shutdown(client_get_fd(cr->clients[i]),SHUT_RDWR);
        }
    }

}