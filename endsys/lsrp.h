#ifndef lsrpHEADER
#define lsrpHEADER
#include "commonItems.h"

struct packet packet_encapsulation(struct data_segment msg);
struct data_segment packet_decapsulation(struct packet pkt);

#endif
