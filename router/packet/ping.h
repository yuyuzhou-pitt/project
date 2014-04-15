#ifndef __PING_H__
#define __PING_H__

#define PING 1
#include "packet.h"

Packet *genPingMsg(Router *router, int ping_pong_bit, struct timeval timer);
int sendPing(int sockfd, Router *router, struct timeval timer);
int sendPong(int sockfd, Router *router, struct timeval timer);
struct timeval calCost(Packet *packet_req, int alpha, struct timeval cost, struct timeval timer);

#endif
