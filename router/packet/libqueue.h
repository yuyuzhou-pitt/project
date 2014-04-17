#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "packet.h"
#include "../config/config.h"
#define ETHX 128
#define DBMAX 1024

typedef struct Packet_Queue{
    Packet *packet;
    struct Packet_Queue *next;
}Packet_Q;

typedef struct Packet_Buffer{
    int buffsize;
    Packet_Q *packet_q;
}Packet_Buff;

/* for thread parameters */
typedef struct Thread_Parameters{
    int sockfd; // record the sockfd for client thread
    int port; // record the port for client thread use
    Router *router;
    int ls_db_size;
    LS_DB ls_db[DBMAX];
    Packet_Buff buffer[ETHX]; // group buffer according to ethx
}ThreadParam;

Packet_Q *initlist();
void enqueue(Packet_Q *head, Packet *v);
Packet_Q *dequeue(Packet_Q *head);
Packet_Buff *initBuff();
int listsize(Packet_Q *head);
int addBufferACK(ThreadParam *threadParam, Packet *packet_ack, int ethx);


#endif
