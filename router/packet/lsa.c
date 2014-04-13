#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define LSA 1
#include "packet.h"

Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer){
    /*generate lsa message*/

    LSA_Msg lsa_msg;

    snprintf(lsa_msg.Advertising_Router_ID, sizeof(lsa_msg.Advertising_Router_ID), "%s", router->router_id);
    lsa_msg.LS_Age = timer.tv_sec;
    lsa_msg.LS_Sequence_Number = ls_sequence_number;
    //lsa_msg.Length = 1//how ??
    lsa_msg.Number_of_Links = router->num_of_interface; 

    int i;
    for(i=0;i < router->num_of_interface;i++){
        snprintf(lsa_msg.ls_link[i].Link_ID, sizeof(lsa_msg.ls_link[i].Link_ID), "%s", router->ethx[i].eth_id);
        lsa_msg.ls_link[i].Availability = router->ethx[i].link_availability; //default as 1, set it as 0 if timeout or disabled
        lsa_msg.ls_link[i].Link_Cost = timer; //router->ethx[i].link_cost;
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

int sendLSA(int sockfd, Router *router, int ls_sequence_number, struct timeval timer){
    Packet *lsa_packet;
    lsa_packet = genLSAMsg(router, ls_sequence_number, timer); // msg to be sent out
    Send(sockfd, lsa_packet, sizeof(Packet), 0);
    return 0;
}
