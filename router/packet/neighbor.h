#ifndef __NIEGHBOR_H__
#define __NIEGHBOR_H__

#define NEIGHBOR 1
#include "packet.h"
#include "../config/config.h"

Packet *genNeighborReq(Router *router, int port);
int genNeighborReply(Router *router, Packet *neighbor_req, Packet *neighbor_reply);
int sendNeighborReply(int sockfd, Packet *packet_req, Router *router, int port);

#endif
