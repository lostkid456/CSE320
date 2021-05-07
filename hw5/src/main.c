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
#include "server.h"
#include "globals.h"

#include "csapp.h"

static void terminate(int);

void sighup_handler(int);

int create_thread(pthread_t *tid,int fd);

pthread_t tid; 

// static pthread_t tids[MAX_CLIENTS];

/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    //Counter to keep track if -p is found
    int p_counter=0;

    //port for -p 
    char* port;

    //used for getopt
    int option;

    //Descriptors for connection between server and client
    int listen_fd;
    socklen_t clients;
    struct sockaddr_in cliaddr;

    while( (option=getopt(argc,argv,"p:") )!=-1){
        switch(option){
            case 'p':
                port=optarg;
                p_counter++;
                break;
            default:
                printf("Unknown/Missing Option\n");
                break;
        }
    }

    if(!(p_counter)){
        fprintf(stderr,"-p not found\n");
        return -1;
    }



    // Perform required initializations of the client_registry and
    // player_registry.
    user_registry = ureg_init();
    client_registry = creg_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    listen_fd=Open_listenfd(port);

    struct sigaction action;
    action.sa_handler=sighup_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags=SA_SIGINFO;
    sigaction(SIGHUP,&action,NULL);

    int conn_fd;

    while(1){
        clients=sizeof(struct sockaddr_storage);
        conn_fd=Accept(listen_fd,(SA*)&cliaddr,&clients);
        create_thread(&tid,conn_fd);
        pthread_detach(tid);
    }
}

int create_thread(pthread_t *tid,int fd){
    int *copy_fd=malloc(sizeof(int));
    *copy_fd=fd;
    return pthread_create(tid,NULL,chla_client_service,copy_fd);
}

void sighup_handler(int signal){
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    // pthread_join(tid,NULL);
    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}
