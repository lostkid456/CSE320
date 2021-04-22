#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include "protocol.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){
    hdr->type=ntohl(hdr->type);
    hdr->payload_length=htonl(hdr->payload_length);
    hdr->msgid=htonl(hdr->msgid);
    hdr->timestamp_sec=htonl(hdr->timestamp_sec);
    hdr->timestamp_nsec=htonl(hdr->timestamp_nsec);
    if(write(fd,hdr,sizeof(hdr))!=-1){
        if(payload!=NULL){
            if(write(fd,payload,hdr->payload_length)!=-1){
                return 0;
            }else{
                return -1;
            }
        }
        return 0;
    }
    return -1;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
    if(read(fd,hdr,sizeof(hdr))!=-1){
        if(htonl(hdr->payload_length)>0){
            if(read(fd,payload,htonl(hdr->payload_length))!=-1){
                return 0;
            }else{
                return -1;
            }
        }
    }
    return -1;
}