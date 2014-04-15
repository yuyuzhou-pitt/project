#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define LSA 1
#include "packet.h"
#include "libqueue.h"

Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer, int port){
//Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer, int port, char *remote_server){
    /*generate lsa message*/

    LSA_Msg lsa_msg;

    snprintf(lsa_msg.Advertising_Router_ID, sizeof(lsa_msg.Advertising_Router_ID), "%s", router->router_id); // original src_ip
    //snprintf(lsa_msg.Destination_Router_ID, sizeof(lsa_msg.Destination_Router_ID), "%s", remote_server); // original des_ip
    lsa_msg.LS_Age = timer.tv_sec;
    lsa_msg.LS_Sequence_Number = ls_sequence_number; // new ls_sequence_number is 0, add 1 when repack
    //lsa_msg.Length = 1//how ??
    lsa_msg.Number_of_Links = router->num_of_interface; 

    int i;
    for(i=0;i < router->num_of_interface;i++){
        snprintf(lsa_msg.ls_link[i].Link_ID, sizeof(lsa_msg.ls_link[i].Link_ID), "%s", router->ethx[i].eth_id);
        snprintf(lsa_msg.ls_link[i].Direct_Link_Addr, sizeof(lsa_msg.ls_link[i].Direct_Link_Addr), "%s", router->ethx[i].direct_link_addr);
        lsa_msg.ls_link[i].PortID = port;
        lsa_msg.ls_link[i].Availability = router->ethx[i].link_availability; //default as 1, set it as 0 if timeout or disabled
        lsa_msg.ls_link[i].Link_Cost = router->ethx[i].link_cost; //router->ethx[i].link_cost;
    }
    /*wrap into packet*/
    Packet *lsa_packet;
    lsa_packet = (Packet *)malloc(sizeof(Packet)); //Packet with LSA_Msg type Data

    snprintf(lsa_packet->RouterID, sizeof(lsa_packet->RouterID), "%s", router->router_id);
    snprintf(lsa_packet->PacketType, sizeof(lsa_packet->PacketType), "%s", "010"); // LSA Packets (010)

    lsa_packet->Data = (LSA_Msg) lsa_msg; // Data

    /*checksum*/
    snprintf(lsa_packet->PacketChecksum, sizeof(lsa_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) lsa_packet, sizeof(*lsa_packet)));

    return lsa_packet;
}

int sendNewLSA(int sockfd, Router *router, int ls_sequence_number, struct timeval timer, int port){
//int sendNewLSA(int sockfd, Router *router, int ls_sequence_number, struct timeval timer, int port, char *remote_server){
    Packet *lsa_packet;
    //lsa_packet = genLSAMsg(router, ls_sequence_number, timer, port, remote_server); // msg to be sent out
    lsa_packet = genLSAMsg(router, ls_sequence_number, timer, port); // msg to be sent out
    Send(sockfd, lsa_packet, sizeof(Packet), 0);
    //printf("sendLSA: lsa_packet->PacketType = %s\n", lsa_packet->PacketType);
    return 0;
}

/* replace the router_id */
int repackLSA(Router *router, Packet *packet){
    snprintf(packet->RouterID, sizeof(packet->RouterID), "%s", router->router_id);
}

int sendBufferLSA(int sockfd, Packet_Buff *buffer){
    Packet *lsa_packet;
    if(buffer->buffsize > 0){
        lsa_packet = (Packet *)dequeue(buffer->packet_q);
        Send(sockfd, lsa_packet, sizeof(Packet), 0);
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

int installLSA(ThreadParam *threadParam, Packet *packet_req, int ethx){
    Packet *packet;
    packet = (Packet *)packet_req;

    /* check whether the same LSA with new sequence number */
    int sameLSA = 0;

    int dbIndex;
    for(dbIndex=0;dbIndex < threadParam->ls_db_size; dbIndex++){
        if(strcmp(threadParam->ls_db[dbIndex].des_router_id, packet->Data.Advertising_Router_ID) == 0){
            sameLSA = 1;
            /* do update if got bigger seq number */
            if(threadParam->ls_db[dbIndex].LS_Sequence_Number < packet->Data.LS_Sequence_Number){
                int i;
                for(i=0;i < packet->Data.Number_of_Links;i++){
                    if(strcmp(threadParam->ls_db[dbIndex].src_router_id, packet->Data.ls_link[i].Direct_Link_Addr) == 0){
                        threadParam->ls_db[dbIndex].Availability = packet->Data.ls_link[i].Availability;
                        threadParam->ls_db[dbIndex].LS_Sequence_Number = packet->Data.LS_Sequence_Number;
                        threadParam->ls_db[dbIndex].Link_Cost = packet->Data.ls_link[i].Link_Cost;
                        threadParam->ls_db[dbIndex].LS_Age = packet->Data.LS_Age;
                    }
                }

            }
        }
    }

    if(sameLSA == 0){
        /* add all new LS into db */
        int i;
        for(i=0;i < packet->Data.Number_of_Links;i++){
            dbIndex = threadParam->ls_db_size;
            snprintf(threadParam->ls_db[dbIndex].Link_ID, sizeof(threadParam->ls_db[dbIndex].Link_ID), "%s", packet->Data.ls_link[i].Link_ID);
            snprintf(threadParam->ls_db[dbIndex].src_router_id, sizeof(threadParam->ls_db[dbIndex].src_router_id), "%s", packet->Data.ls_link[i].Direct_Link_Addr);
            threadParam->ls_db[dbIndex].des_port_id = packet->Data.ls_link[i].PortID;
            snprintf(threadParam->ls_db[dbIndex].des_router_id, sizeof(threadParam->ls_db[dbIndex].des_router_id), "%s", packet->Data.Advertising_Router_ID);
            threadParam->ls_db[dbIndex].src_port_id = packet->Data.ls_link[i].PortID;
            threadParam->ls_db[dbIndex].Availability = packet->Data.ls_link[i].Availability;
            threadParam->ls_db[dbIndex].LS_Sequence_Number = packet->Data.LS_Sequence_Number;
            threadParam->ls_db[dbIndex].Link_Cost = packet->Data.ls_link[i].Link_Cost;
            threadParam->ls_db[dbIndex].LS_Age = packet->Data.LS_Age;
    
            threadParam->ls_db_size++;
        }
    }

    return 0;
}

/* add LSA to flood buffer, except the ethx from which it received the LSA. */
int addBufferFlood(ThreadParam *threadParam, Packet *packet, int ethx){
    int i;
    int isDirect = 0;
    //printf("addBufferflood:threadParam->router->num_of_interface = %d\n",threadParam->router->num_of_interface);
    for (i=0;i < threadParam->router->num_of_interface; i++){
        /* prevent loop before enqueue: original Advertising_Router_ID is NOT in direct_link_addr */
        if(strcmp(threadParam->router->ethx[i].direct_link_addr, packet->Data.Advertising_Router_ID) == 0){
            isDirect = 1;
        }
    }
    if(isDirect == 0){
        for (i=0;i < threadParam->router->num_of_interface; i++){
            if(i != ethx){
                enqueue(threadParam->buffer[i].packet_q, packet);
                //printf("addBufferflood:threadParam->buffer[i].packet_q->next->packet->RouterID = %s\n",threadParam->buffer[i].packet_q->next->packet->RouterID);
                threadParam->buffer[i].buffsize++;
                //printf("addBufferflood:threadParam->buffer[i].buffsize = %d\n",threadParam->buffer[i].buffsize);
            }
        }
    }
    return 0;
}
