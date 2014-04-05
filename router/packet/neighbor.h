#ifndef __NIEGHBOR_H__
#define __NIEGHBOR_H__

#define NEIGHBOR 1
#include "packet.h"

Packet *genNeighborReq(char *filename, char *hostip, int port);
int genNeighborReply(char *filename, Packet *neighbor_req, Packet *neighbor_reply);

#endif
