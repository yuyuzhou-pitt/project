#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define PING 1
#include "packet.h"

Packet *genPingMsg(Router *router, int ping_pong_bit, struct timeval timer){
    /*generate ping message*/

    Ping_Msg ping_msg;

    ping_msg.ping_pong_bit = ping_pong_bit;
    ping_msg.timer = timer;
    ping_msg.packet_life = router->ping_timeout; // >= the maximum cost

    /*wrap into packet*/
    Packet *ping_packet;
    ping_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Ping_Msg type Data

    snprintf(ping_packet->RouterID, sizeof(ping_packet->RouterID), "%s", router->router_id);
    snprintf(ping_packet->PacketType, sizeof(ping_packet->PacketType), "%s", "011"); // Ping Packets (011)

    ping_packet->Data = (Ping_Msg) ping_msg; // Data

    /*checksum*/
    snprintf(ping_packet->PacketChecksum, sizeof(ping_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) ping_packet, sizeof(*ping_packet)));

    return ping_packet;
}

int sendPing(int sockfd, Router *router, struct timeval timer){
    Packet *ping_packet;
    ping_packet = genPingMsg(router, '0', timer); // msg to be sent out, 0 means request 
    Send(sockfd, ping_packet, sizeof(Packet), 0);
}

int sendPong(int sockfd, Router *router, struct timeval timer){
    Packet *pong_packet;
    pong_packet = genPingMsg(router, '1', timer); // 1 means pong
    Send(sockfd, pong_packet, sizeof(Packet), 0);
}

struct timeval calCost(Packet *packet_req, struct timeval timer){
    struct timeval cost;

    /* calculate link cost */
    cost.tv_sec = timer.tv_sec - packet_req->Data.timer.tv_sec;
    cost.tv_usec = timer.tv_usec - packet_req->Data.timer.tv_usec;

    /* update routing table */

    return cost;
}
