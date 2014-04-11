#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "packet.h"

typedef struct Packet_Queue{
    Packet *packet;
    struct Packet_Queue *next;
}Packet_Q;

typedef struct Packet_Buffer{
    int buffsize;
    Packet_Q *packet_q;
}Packet_Buff;

Packet_Q *initlist();
void enqueue(Packet_Q *head, Packet *v);
Packet_Q *dequeue(Packet_Q *head);
Packet_Buff *initBuff();
int listsize(Packet_Q *head);

#endif
