#ifndef lsrpHEADER
#define lsrpHEADER
#include "commonItems.h"
#include "sw.h"
#include "socket.h"

void lsrp_outgoingmessage(struct data_segment ds, int size);
void lsrp_incomingmessage(struct packet pkt);

struct packet packet_encapsulation(struct data_segment ds, int size);
struct data_segment packet_decapsulation(struct packet pkt);

void lsrp_readInCfg();

#endif
