#ifndef swHEADER
#define swHEADER
#include "commonItems.h"
#include "app.h"
#include "lsrp.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct data_segment outgoing_buffer[10];
struct message incoming_buffer[10];

void sw_outgoingmessage(struct message msg, int size, char * IP);
int sw_incomingmessage(struct data_segment ds);
struct data_segment sw_getACK();

struct data_segment segment_encapsulation(struct message msg, int size);
struct message segment_decapsulation(struct data_segment ds);
void sw_readInCfg();

#endif
