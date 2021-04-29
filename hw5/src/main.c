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
    int listen_fd,*conn_fd;
    socklen_t clients;
    struct sockaddr_in cliaddr;
    pthread_t tid; 

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

    signal(SIGHUP,sighup_handler);
    while(1){
        clients=sizeof(struct sockaddr_storage);
        conn_fd=malloc(sizeof(int));
        *conn_fd=Accept(listen_fd,(SA*)&cliaddr,&clients);
        pthread_create(&tid,NULL,chla_client_service,conn_fd);
    }
}

void sighup_handler(int signal){
    //exit(0);
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}
