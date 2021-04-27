#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "user.h"
#include "globals.h"

typedef struct user{
    char* handle;
    volatile int reference_count;
    char* why;
    sem_t mutex;
}USER;

USER *user_create(char *handle){
    USER *new_user=malloc(sizeof(USER));
    sem_init(&new_user->mutex,0,1);
    new_user->handle=malloc(sizeof(char));
    strcpy(new_user->handle,handle);
    new_user->reference_count=1;
    new_user->why=NULL;
    return new_user;
}

USER *user_ref(USER *user, char *why){
    sem_wait(&user->mutex);
    (user->reference_count)+=1;
    user->why=why;
    sem_post(&user->mutex);
    return user;
}

void user_unref(USER *user, char *why){
    sem_wait(&user->mutex);
    (user->reference_count)-=1;
    user->why=why;
    if(user->reference_count==0){
        free(user->handle);
        free(user);
    }
    sem_post(&user->mutex);
}

char *user_get_handle(USER *user){
    return user->handle;
}