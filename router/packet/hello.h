#ifndef __HELLO_H__
#define __HELLO_H__

#define HELLO 1
#include "packet.h"
#include "../config/config.h"

Packet *genHelloReq(Router *router, int port);
int sendHello(int sockfd, Router *router, int port);

#endif
