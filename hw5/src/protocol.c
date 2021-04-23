#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include "protocol.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){
    size_t packet_len=ntohl(hdr->payload_length);
    if(write(fd,hdr,sizeof(*hdr))==sizeof(*hdr)){
        if(payload!=NULL){
            if(write(fd,payload,packet_len)==packet_len){
                return 0;
            }else{
                errno=EIO;
                return -1;
            }
        }
        return 0;
    }
    errno=EIO;
    return -1;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
    //memset(hdr,0,sizeof(*hdr));
    size_t packet_len;
    if(read(fd,hdr,sizeof(*hdr))==sizeof(*hdr)){
        packet_len=ntohl(hdr->payload_length);
        char* hold_payload=malloc(packet_len);
        char* temp_payload=hold_payload;
        int counter=packet_len;
        while(counter){
            size_t rv=read(fd,temp_payload,counter);
            if(rv<0 || rv==0){
                free(hold_payload);
                return -1;
            }
            counter-=rv;
            temp_payload+=counter;
        }
        if(payload!=NULL){
            *payload=hold_payload;
        }else{
            free(hold_payload);
        }
        return 0;
    }
    errno=EIO;
    return -1;
}