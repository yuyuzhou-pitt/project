#ifndef lsrpHEADER
#define lsrpHEADER
#include "commonItems.h"
#include "sw.h"
#include "socket.h"

void lsrp_outgoingmessage(struct data_segment ds, int size, char * IP);
int lsrp_incomingmessage(struct packet pkt);

struct packet packet_encapsulation(struct data_segment ds, int size, char * IP);
struct data_segment packet_decapsulation(struct packet pkt);
struct packet lsrp_createACK(struct packet recv);

void lsrp_readInCfg();

#endif
