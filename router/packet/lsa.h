#ifndef __LSA_H__
#define __LSA_H__

#define LSA 1
#include "packet.h"

Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer, int port);
int sendLSA(int sockfd, Router *router, int ls_sequence_number, struct timeval timer, int port);

#endif
