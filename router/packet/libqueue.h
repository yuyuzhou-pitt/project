#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <pthread.h>
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
    pthread_mutex_t lock_buffer;
    Packet_Q *packet_q;
}Packet_Buff;

/* for thread parameters */
typedef struct Thread_Parameters{
    int sockfd; // record the sockfd for client thread
    int port; // record the port for client thread use
    Router *router;
    Packet_Buff lsa_buffer[ETHX]; // group buffer according to ethx
    Packet_Buff data_buffer[ETHX]; // group buffer according to ethx
    int ls_db_size;
    LS_DB ls_db[DBMAX]; // recording the whole ls db
    int graph_line_size;
    Graph_Line graph_line[ETHX];
    int routing_size;
    Routing_Table routing[ETHX]; // recording local routing table

    pthread_mutex_t lock_send;
    pthread_mutex_t lock_port;
    pthread_mutex_t lock_router;
    pthread_mutex_t lock_graph;

}ThreadParam;

Packet_Q *initlist();
void enqueue(Packet_Q *head, Packet *v);
Packet *dequeue(Packet_Q *head);
Packet_Buff *initBuff();
int listsize(Packet_Q *head);
int addBufferACK(ThreadParam *threadParam, Packet *packet_ack, int ethx);


#endif
