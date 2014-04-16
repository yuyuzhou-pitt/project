#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define LSA 1
#include "packet.h"
#include "libqueue.h"

Packet *genLSAMsg(ThreadParam *threadParam, int ls_sequence_number, struct timeval timer){
//Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer, int port, char *remote_server){
    /*generate lsa message*/
    Router *router;
    router = threadParam->router;

    LSA_Msg lsa_msg;

    //printf("genLSAMsg: set Advertising_Router_ID: %s\n", router->router_id);
    snprintf(lsa_msg.Advertising_Router_ID, sizeof(lsa_msg.Advertising_Router_ID), "%s", router->router_id); // original src_ip
    //snprintf(lsa_msg.Destination_Router_ID, sizeof(lsa_msg.Destination_Router_ID), "%s", remote_server); // original des_ip
    lsa_msg.LS_Age = timer.tv_sec;
    lsa_msg.LS_Sequence_Number = ls_sequence_number; // new ls_sequence_number is 0, add 1 when repack
    //lsa_msg.Length = 1//how ??
    lsa_msg.Number_of_Links = router->num_of_interface; 

    int i;
    for(i=0;i < router->num_of_interface;i++){
        //printf("genLSAMsg: set port id: %d\n", port);
        snprintf(lsa_msg.ls_link[i].Link_ID, sizeof(lsa_msg.ls_link[i].Link_ID), "%s", router->ethx[i].eth_id);
        snprintf(lsa_msg.ls_link[i].Direct_Link_Addr, sizeof(lsa_msg.ls_link[i].Direct_Link_Addr), "%s", router->ethx[i].direct_link_addr);
        lsa_msg.ls_link[i].PortID = threadParam->port;
        lsa_msg.ls_link[i].Availability = router->ethx[i].link_availability; //default as 1, set it as 0 if timeout or disabled
        lsa_msg.ls_link[i].Link_Cost = router->ethx[i].link_cost; //router->ethx[i].link_cost;
    }
    /*wrap into packet*/
    Packet *lsa_packet;
    lsa_packet = (Packet *)calloc(1, sizeof(Packet)); //Packet with LSA_Msg type Data

    snprintf(lsa_packet->RouterID, sizeof(lsa_packet->RouterID), "%s", router->router_id);
    snprintf(lsa_packet->PacketType, sizeof(lsa_packet->PacketType), "%s", "010"); // LSA Packets (010)

    lsa_packet->Data = (LSA_Msg) lsa_msg; // Data

    /*checksum*/
    snprintf(lsa_packet->PacketChecksum, sizeof(lsa_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) lsa_packet, sizeof(*lsa_packet)));

    return lsa_packet;
}

int installLSA(ThreadParam *threadParam, Packet *packet_req){
    Packet *packet;
    packet = (Packet *)packet_req;

    /* check whether the same LSA with new sequence number */
    int sameLSA = 0;

    int dbIndex;
    //printf("installLSA: got LSA packet from: %s\n", packet->Data.Advertising_Router_ID);
    for(dbIndex=0;dbIndex < threadParam->ls_db_size; dbIndex++){
        /* whether Advertising_Router_ID is the des_router_id */
        if(strcmp(threadParam->ls_db[dbIndex].des_router_id, packet->Data.Advertising_Router_ID) == 0){
            sameLSA = 1;
            /* do update if got bigger seq number */
            //printf("installLSA: this packet from %s in db already\n", packet->Data.Advertising_Router_ID);
            if(threadParam->ls_db[dbIndex].LS_Sequence_Number < packet->Data.LS_Sequence_Number){
                //printf("installLSA: do update as got bigger seq number %d from: %s\n", packet->Data.LS_Sequence_Number, packet->Data.Advertising_Router_ID);
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
        //printf("installLSA: install new LSA packet from %s\n", packet->Data.Advertising_Router_ID);
        /* add all new LS into db */
        int i;
        for(i=0;i < packet->Data.Number_of_Links;i++){
            //printf("installLSA: Advertising_Router_ID - PortID: %s - %s\n", packet->Data.Advertising_Router_ID, packet->Data.ls_link[i].PortID);
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

int sendNewLSA(int sockfd, ThreadParam *threadParam, int ls_sequence_number, struct timeval timer, int port){
//int sendNewLSA(int sockfd, Router *router, int ls_sequence_number, struct timeval timer, int port, char *remote_server){
    Packet *lsa_packet;
    //lsa_packet = genLSAMsg(router, ls_sequence_number, timer, port, remote_server); // msg to be sent out

    lsa_packet = genLSAMsg(threadParam, ls_sequence_number, timer); // msg to be sent out
    /* add into LS DB before send out */
    installLSA(threadParam, lsa_packet);
    //printf("sendNewLSA: send LSA packet %s with type %s\n", lsa_packet->RouterID, lsa_packet->PacketType);
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

/* for terminal */
int showLSDB(ThreadParam *threadParam){
    int i;
    printf("\nshowLSDB: ls_db_size: %d\n", threadParam->ls_db_size);
    printf("Link_ID            \tsrc_router\tdes_router\tport\tAvail\tSeq\tCost\tLS_Age    \n");
    /*  10.0.0.1	10.0.0.4	10.0.0.5	36747	1	3	9999:0	1397584710 */
    for(i=0;i < threadParam->ls_db_size;i++){
        printf("%s\t%s\t%s\t%d\t%d\t%d\t%d:%d\t%d\n", threadParam->ls_db[i].Link_ID, threadParam->ls_db[i].src_router_id, \
               threadParam->ls_db[i].des_router_id, threadParam->ls_db[i].des_port_id, threadParam->ls_db[i].Availability, \
               threadParam->ls_db[i].LS_Sequence_Number, threadParam->ls_db[i].Link_Cost.tv_sec, \
               threadParam->ls_db[i].Link_Cost.tv_usec, threadParam->ls_db[i].LS_Age);
    }
    return 0;
}
