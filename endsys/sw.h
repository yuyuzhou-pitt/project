#ifndef swHEADER
#define swHEADER
#include "commonItems.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct data_segment outgoing_buffer[10];
struct message incoming_buffer[10];

struct data_segment segment_encapsulation(struct message msg);
struct message segment_decapsulation(struct data_segment ds);
void sw_readInCfg();

#endif
