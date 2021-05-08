#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "debug.h"
#include "client_registry.h"
#include "client.h"

static sem_t lock_shutdown;
static sem_t hold_shutdown;

typedef struct client_registry{
    CLIENT **clients;
    volatile int total_clients;
    sem_t mutex;
}CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *client_registry=malloc(sizeof(CLIENT_REGISTRY));
    if(client_registry==NULL){
        return NULL;
    }
    client_registry->clients=calloc(MAX_CLIENTS,sizeof(CLIENT*));
    client_registry->total_clients=0;
    sem_init(&client_registry->mutex,0,1);
    sem_init(&lock_shutdown,0,1);
    sem_init(&hold_shutdown,0,1);
    debug("Initialize client registry");
    return client_registry;
}

void creg_fini(CLIENT_REGISTRY *cr){
    sem_wait(&cr->mutex);
    free(cr->clients);
    sem_post(&cr->mutex);
    sem_destroy(&cr->mutex);
    free(cr);
    debug("Finishing client registry");
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
            break;
        }
    }
    sem_post(&cr->mutex);
    return new_client;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client){
    sem_wait(&cr->mutex);
    debug("Creg_unregister is called");
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]!=NULL){
            if(client_get_fd(cr->clients[i])==client_get_fd(client)){
                client_unref(cr->clients[i],"Dereferencing");
                //Added to set to NULL
                cr->clients[i]=NULL;
                //debug("Unreferencing client %i",client_get_fd(cr->clients[i]));
                (cr->total_clients)-=1;
                if(cr->total_clients==0){
                    sem_post(&hold_shutdown);
                    debug("OPENED SINCE NO MORE CLIENTS");
                }
                sem_post(&cr->mutex);
                sem_post(&hold_shutdown);
                return 0;
            }
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
            client_list[i]=client_ref(cr->clients[i],"Increment reference");
            debug("Incrementing reference for creg_all_clients");
        }
    }
    sem_post(&cr->mutex);
    return client_list;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    sem_wait(&lock_shutdown);
    if(cr->total_clients==0){
        debug("No more clients");
        sem_post(&lock_shutdown);
        return;
    }
    for(int i=0;i<MAX_CLIENTS;i++){
        if(cr->clients[i]!=NULL){
            if(client_get_fd(cr->clients[i])!=0){
                shutdown(client_get_fd(cr->clients[i]),SHUT_RDWR);
                debug("Shutdown client index %i",i);
                //debug("Shutdown fd:%i",client_get_fd(cr->clients[i]));
            }
        }else{
            break;
        }
    }
    while(cr->total_clients!=0){
        debug("Blocked creg_shutdown_all()");
        sem_wait(&hold_shutdown);
    }
    sem_post(&lock_shutdown);
    sem_post(&hold_shutdown);
    return;
}