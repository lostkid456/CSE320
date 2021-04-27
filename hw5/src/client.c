#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#include "client.h"

typedef struct client{
    int fd;
    volatile int ref_count;
    USER *user;
    MAILBOX *mailbox;
    pthread_mutex_t r_lock;
}CLIENT;

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd){

}

CLIENT *client_ref(CLIENT *client, char *why){

}

void client_unref(CLIENT *client, char *why){

}

int client_login(CLIENT *client, char *handle){

}

int client_logout(CLIENT *client){

}

USER *client_get_user(CLIENT *client, int no_ref){

}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref){

}

int client_get_fd(CLIENT *client){

}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data){

}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen){

}

int client_send_nack(CLIENT *client, uint32_t msgid){

}