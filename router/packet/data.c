#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/libfile.h"
#include "../config/liblog.h"

#define TRANSFILE 1
#include "packet.h"
#include "libqueue.h"

int sendBufferData(int sockfd, Packet_Buff *buffer){
    Packet *packet;
    if(buffer->buffsize > 0){
        packet = (Packet *)dequeue(buffer->packet_q);
        Send(sockfd, packet, sizeof(Packet), 0);
        buffer->buffsize--;
    }
    else{
        char logmsg[128]; 
        snprintf(logmsg, sizeof(logmsg), "sendBufferLSA: Buffer is empty.");
        logging(LOGFILE, logmsg);
        return -1;
    }
    return 0;
}

int addBufferData(ThreadParam *threadParam, Packet *packet){
    
    int i,j;
    for(i=0;i < threadParam->routing_size;i++){
        /* found detination in routing table */
        if(strcmp(packet->Data.des_ip, threadParam->routing[i].Destination) == 0){
            /* found interface for buffer index */
            for(j=0;j< threadParam->router->num_of_interface;j++){
                if(strcmp(threadParam->routing[i].Interface, threadParam->router->ethx[j].eth_id) == 0){
                    enqueue(threadParam->buffer[j].packet_q, packet);
                    threadParam->buffer[j].buffsize++;

                    return 0;
                }
            }
        }
    }

    return -1; // if no routing
}
