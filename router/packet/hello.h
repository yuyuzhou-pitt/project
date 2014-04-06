#ifndef __HELLO_H__
#define __HELLO_H__

#define HELLO 1
#include "packet.h"

Packet *genHelloReq();
void *hellothread(void *arg);

#endif
