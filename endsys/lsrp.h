#ifndef lsrpHEADER
#define lsrpHEADER
#include "commonItems.h"
#include "sw.h"

void lsrp_outgoingmessage(struct data_segment ds);
void lsrp_incomingmessage(struct packet pkt);

struct packet packet_encapsulation(struct data_segment ds);
struct data_segment packet_decapsulation(struct packet pkt);

#endif
