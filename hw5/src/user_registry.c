#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "debug.h"
#include "user_registry.h"
#include "user.h"

typedef struct user_registry{
    USER *user;
    USER_REGISTRY *next;
    USER_REGISTRY *prev;
    sem_t mutex;
} USER_REGISTRY;

USER_REGISTRY *ureg_init(void){
    USER_REGISTRY *user_registry=malloc(sizeof(USER_REGISTRY));
    if(user_registry==NULL){
        return NULL;
    }
    user_registry->next=user_registry;
    user_registry->prev=user_registry;
    sem_init(&user_registry->mutex,0,1);
    debug("Initalize user registry");
    return user_registry;
}

void ureg_fini(USER_REGISTRY *ureg){
    sem_wait(&ureg->mutex);
    USER_REGISTRY *head=ureg->next;
    while(head!=ureg){
        free(user_get_handle(head->user));
        free(head->user);
        head=head->next;
    }
    sem_post(&ureg->mutex);
    free(head);
}

USER *ureg_register(USER_REGISTRY *ureg, char *handle){
    if(ureg==NULL){
        return NULL;
    }
    USER *user;
    sem_wait(&ureg->mutex);
    USER_REGISTRY *head=ureg->next;
    while(head!=ureg){
        if(strcmp(user_get_handle(head->user),handle)==0){
            user=user_ref(head->user,"Increasing reference for already registered user");
            sem_post(&ureg->mutex);
            return user;
        }
        head=head->next;
    }
    user=user_create(handle);
    if(user==NULL){
        sem_post(&ureg->mutex);
        return NULL;
    }
    user_ref(user,"Incresing reference");
    USER_REGISTRY *next_user=malloc(sizeof(USER_REGISTRY));
    next_user->user=user;
    next_user->next=head->next;
    next_user->prev=head;
    head->next->prev=next_user;
    head->next=next_user;
    sem_post(&ureg->mutex);
    return user;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle){
    sem_wait(&ureg->mutex);
    USER_REGISTRY *head=ureg->next;
    while(head!=ureg){
        if(strcmp(user_get_handle(head->user),handle)==0){
            user_unref(head->user,"Decreasing reference of user");
            if(head->user==NULL){
                head->prev->next=head->next;
                head->next->prev=head->prev;
                sem_post(&ureg->mutex);
                free(head);
                return;
            }
        }
    }
    sem_post(&ureg->mutex);
}
