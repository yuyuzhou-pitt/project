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
    //printf("sendPing: ping_packet->PacketType = %s\n", ping_packet->PacketType);

    return 0;
}

int sendPong(int sockfd, Router *router, struct timeval timer){
    Packet *pong_packet;
    pong_packet = genPingMsg(router, '1', timer); // 1 means pong
    Send(sockfd, pong_packet, sizeof(Packet), 0);

    return 0;
}

struct timeval *calCost(Packet *packet_req, int alpha, struct timeval *cost, struct timeval timer){
    struct timeval current_cost, *new_cost;

    //printf("calCost: my timer is: %d:%d\n", timer.tv_sec, timer.tv_usec);
    //printf("calCost: remote timer is: %d:%d\n", packet_req->Data.timer.tv_sec, packet_req->Data.timer.tv_usec);
    /* calculate link cost */
    current_cost.tv_sec = timer.tv_sec - packet_req->Data.timer.tv_sec;
    current_cost.tv_usec = timer.tv_usec - packet_req->Data.timer.tv_usec;

    //printf("calCost: raw cost is: %d:%d\n", current_cost.tv_sec, current_cost.tv_usec);
    /* update routing table */

    cost->tv_sec = alpha * cost->tv_sec + (1 -  alpha) * current_cost.tv_sec;
    cost->tv_usec = alpha * cost->tv_usec + (1 -  alpha) * current_cost.tv_usec;
    printf("calCost: real cost is: %d:%d\n", cost->tv_sec, cost->tv_usec);
    
    return cost;
}
