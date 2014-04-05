#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"

#define NEIGHBOR 1
#include "packet.h"


Packet *genNeighborReq(char *filename, char *hostip, int port){
    /*generate neighbor message*/
    Neighbor_Msg neighbor_msg;
    //Neighbor_Msg *neighbor_req;
    //neighbor_req = (Neighbor_Msg *)malloc(sizeof(neighbor_req));

    snprintf(neighbor_msg.NeighborAcqType, sizeof(neighbor_msg.NeighborAcqType), "%s", "000"); // Be_Neighbors_Request(000)
    snprintf(neighbor_msg.PortID, sizeof(neighbor_msg.PortID), "%d", port);
    cfgread(filename, "ls_updated_interval", neighbor_msg.HelloInterval);
    cfgread(filename, "protocol_version", neighbor_msg.ProtocolVersion);

    /*wrap into packet*/
    //Packet neighbor_packet;
    Packet *neighbor_packet;
    neighbor_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Neighbor_Msg type Data

    snprintf(neighbor_packet->RouterID, sizeof(neighbor_packet->RouterID), "%s", hostip); // RouterID, use host ip as RouterID
    snprintf(neighbor_packet->PacketType, sizeof(neighbor_packet->PacketType), "%s", "000"); // Neighbor Acquisition Packets (000)
    neighbor_packet->Data = (Neighbor_Msg) neighbor_msg; // Data

    /*checksum*/
    snprintf(neighbor_packet->PacketChecksum, sizeof(neighbor_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) neighbor_packet, sizeof(*neighbor_packet)));

    return neighbor_packet;
}

int genNeighborReply(char *filename, Packet *neighbor_req, Packet *neighbor_reply){
    char interface_num[4];
    Neighbor_Msg neighbor_rep = neighbor_reply->Data;

    snprintf(neighbor_rep.NeighborAcqType, sizeof(neighbor_rep.NeighborAcqType), "%s", "111"); // default is Be_Neighbors_Refuse (111)

    cfgread(filename, "num_of_interface", interface_num); // got the interface number
    /*send confirm if remote ip (RouteID) is in one of the direct link*/
    int i;
    for(i=0;i<atoi(interface_num);i++){
        char eth_x_direct_link_addr[32];
        char direct_link_addr[32];
        sprintf(eth_x_direct_link_addr, "eth_%d_direct_link_addr", i);
        cfgread(filename, eth_x_direct_link_addr, direct_link_addr);
        if (direct_link_addr == neighbor_req->RouterID){
            /*send confirm if req router_id is one of the direct link*/
            snprintf(neighbor_rep.NeighborAcqType, sizeof(neighbor_rep.NeighborAcqType), "%s", "001"); // Be_Neighbors_Confirm (001)
            break;
        }
    }

    return 0;
}
