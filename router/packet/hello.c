#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define HELLO 1
#include "packet.h"

Packet *genHelloReq(Router *router, int port){
    /*generate hello message*/

    Hello_Msg hello_msg;
    hello_msg.PortID = port; // the only data in Hello Message

    /*wrap into packet*/
    Packet *hello_packet;
    hello_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Hello_Msg type Data

    snprintf(hello_packet->RouterID, sizeof(hello_packet->RouterID), "%s", router->router_id);
    snprintf(hello_packet->PacketType, sizeof(hello_packet->PacketType), "%s", "001"); // Hello Packets (001)

    hello_packet->Data = (Hello_Msg) hello_msg; // Data
    //printf("PortID=%d", hello_packet->Data.PortID);

    /*checksum*/
    snprintf(hello_packet->PacketChecksum, sizeof(hello_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) hello_packet, sizeof(*hello_packet)));

    return hello_packet;
}

int sendHello(int sockfd, Router *router, int port){
    /* generate hellos_reply reply according to configure file */
    Packet *hello_packet;
    hello_packet = genHelloReq(router, port); // msg to be sent back
    //printf("sendHello: hello_packet->PacketType = %s\n", hello_packet->PacketType);
    return Send(sockfd, hello_packet, sizeof(Packet), MSG_NOSIGNAL);
}
