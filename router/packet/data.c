#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
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
        //printf("sendBufferData: packet sent: %s\n", packet->RouterID);
        Send(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL);
        buffer->buffsize--;
        //printf("sendBufferData: buffer->buffsize = %d\n", buffer->buffsize);
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
    char target_router[32];
    char logmsg[128];
    memset(target_router, 0, sizeof(target_router));
    //printf("addBufferData: packet->Data.des_ip :%s\n",  packet->Data.des_ip);

    /* check target is end-system-ethx or not */
    int k;
    for(k=0;k< threadParam->router->num_of_interface;k++){
        if(strcmp(packet->Data.des_ip, threadParam->router->ethx[k].direct_link_addr) == 0){
            snprintf(logmsg, sizeof(logmsg), "addBufferData: packet %s enqueued into endsystem buffer[%d].packet_q\n", packet->RouterID, k);
            logging(LOGFILE, logmsg);
            enqueue(threadParam->data_buffer[k].packet_q, packet);
            threadParam->data_buffer[k].buffsize++;
            //printf("addBufferData:(to endsys) buffer[%d].buffsize = %d\n",k, threadParam->data_buffer[k].buffsize);

            return 0;
        }
    }

    /* found destination router from ls_db first */
    for(k=0;k < threadParam->ls_db_size;k++){
        if(strcmp(packet->Data.des_ip, threadParam->ls_db[k].src_router_id) == 0){
            if(strcmp(threadParam->ls_db[k].Link_ID, "end-system-ethx ") == 0){
                strncpy(target_router, threadParam->ls_db[k].des_router_id, strlen(threadParam->ls_db[k].des_router_id));
            }
            else{
                snprintf(logmsg, sizeof(logmsg), "addBufferData: ERROR: destination %s is not end system.", packet->Data.des_ip);
                logging(LOGFILE, logmsg);
                return -1;
            }
        }
    }

    /* add into buffer according to target router */
    int i,j;
    for(i=0;i < threadParam->routing_size;i++){
        /* found detination in routing table */
        if(strcmp(target_router, threadParam->routing[i].Destination) == 0){
            /* found interface for buffer index */
            for(j=0;j< threadParam->router->num_of_interface;j++){
                if(strcmp(threadParam->routing[i].Interface, threadParam->router->ethx[j].eth_id) == 0){
                    snprintf(logmsg, sizeof(logmsg), "addBufferData: packet %s enqueued into threadParam->data_buffer[%d].packet_q\n", packet->RouterID, j);
                    logging(LOGFILE, logmsg);
                    enqueue(threadParam->data_buffer[j].packet_q, packet);
                    threadParam->data_buffer[j].buffsize++;
                    //printf("addBufferData:(to router)buffer[%d].buffsize = %d\n",j, threadParam->data_buffer[j].buffsize);

                    return 0;
                }
            }
        }
    }

    return -1; // if no routing
}

/* replace the router_id */
int repackData(Router *router, Packet *packet){
    snprintf(packet->RouterID, sizeof(packet->RouterID), "%s", router->router_id);
}
