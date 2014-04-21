#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../config/config.h"
#include "../config/liblog.h"
#include "../../endsys/checksum.h"

#define NEIGHBOR 1
#include "packet.h"

Packet *genNeighborReq(Router *router, int port){
    /*generate neighbor message*/
    Neighbor_Msg neighbor_msg; // MUST not be pointer as to be send remote
    //Neighbor_Msg *neighbor_req;
    //neighbor_req = (Neighbor_Msg *)malloc(sizeof(neighbor_req));

    snprintf(neighbor_msg.NeighborAcqType, sizeof(neighbor_msg.NeighborAcqType), "%s", "000"); // Be_Neighbors_Request(000)
    neighbor_msg.PortID = port;
    neighbor_msg.HelloInterval = router->hello_interval;
    neighbor_msg.UpdateInterval = router->ls_updated_interval;
    snprintf(neighbor_msg.ProtocolVersion, sizeof(neighbor_msg.ProtocolVersion), "%s", router->protocol_version);

    /*wrap into packet*/
    //Packet neighbor_packet;
    Packet *neighbor_packet; // could be return as a pointer
    neighbor_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Neighbor_Msg type Data

    snprintf(neighbor_packet->RouterID, sizeof(neighbor_packet->RouterID), "%s", router->router_id); // RouterID, use host ip as RouterID
    snprintf(neighbor_packet->PacketType, sizeof(neighbor_packet->PacketType), "%s", "000"); // Neighbor Acquisition Packets (000)
    neighbor_packet->Data = (Neighbor_Msg) neighbor_msg; // Data

    /*checksum*/
    snprintf(neighbor_packet->PacketChecksum, sizeof(neighbor_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) neighbor_packet, sizeof(*neighbor_packet)));

    return neighbor_packet;
}

int genNeighborReply(Router *router, Packet *neighbor_req, Packet *neighbor_reply){
    snprintf(neighbor_reply->Data.NeighborAcqType, sizeof(neighbor_reply->Data.NeighborAcqType), "%s", "111"); // default is Be_Neighbors_Refuse (111)

    /*send confirm if remote ip (RouteID) is in one of the direct link*/
    int i;
    for(i=0;i<ETHX;i++){ // ETHX defined in packet.h as interfaces number
        if (strcmp(router->ethx[i].direct_link_addr, neighbor_req->RouterID) == 0){
            /*send confirm if req router_id is one of the direct link*/
            snprintf(neighbor_reply->Data.NeighborAcqType, sizeof(neighbor_reply->Data.NeighborAcqType), "%s", "001"); // Be_Neighbors_Confirm (001)
            break;
        }
    }

    return 0;
}

int sendNeighborReply(int sockfd, Packet *packet_req, Router *router, int port){
   /* generate neighbors_reply reply according to configure file */
   Packet *packet_reply;
   packet_reply = genNeighborReq(router, port); // msg to be sent back
   genNeighborReply(router, packet_req, packet_reply); // update the Neighbor Acquisition Type
   return Send(sockfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);
}
