#include <stdlib.h>
#include <stdio.h>

#include "libqueue.h"

/*list initialization*/
Packet_Q *initlist(){
    Packet_Q *head, *z;
    head = (Packet_Q *)malloc(sizeof *head); //head->next->key is the first item
    z = (Packet_Q *)malloc(sizeof *head); //dummy rear, to protect dequeue from an empty list
    head->next = z;
    z->next = z; 
    return head;
}

/*enqueue a thread node to the end*/
void enqueue(Packet_Q *head, Packet *v){
    Packet_Q *t, *r, *z;
    r = head;
    z = head->next; // to find the rear node
    while(z->next!=z){
        r = r->next; // to find the node before rear node
        z = r->next;
    }
    t = (Packet_Q *)malloc(sizeof *t);
    t->packet = v;
    t->next = z;
    r->next = t; //add the node to the end
}

/*dequeue the first thread node*/
Packet *dequeue(Packet_Q *head){
    Packet_Q *r;
    r = head->next;
    head->next = head->next->next;
    return r->packet;
}

/*Buff initialization*/
Packet_Buff *initBuff(){
    Packet_Buff *packet_buff;
    packet_buff = (Packet_Buff *)malloc(sizeof(Packet_Buff));
    packet_buff->buffsize = 0;
    packet_buff->packet_q = initlist();

    return packet_buff;
}

/*get the list size*/
int listsize(Packet_Q *head){
    int len = 0;
    Packet_Q *r;
    r = head->next;
    while(r->next!=r){
        r = r->next;
        len++;
    }
    return len;
}

/* add LSA ack into buffer */
int addBufferACK(ThreadParam *threadParam, Packet *packet_ack, int ethx){
    enqueue(threadParam->buffer[ethx].packet_q, packet_ack);
    threadParam->buffer[ethx].buffsize++;
    return 0;
}
