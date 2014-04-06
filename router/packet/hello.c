#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"

#define HELLO 1
#include "packet.h"

#include "../packet/hello.h"

Packet *genHelloReq(){
    /*generate hello message*/
    char *filename = "../config/lsrp-router.cfg";

    Hello_Msg hello_msg;
    hello_msg.Hello = '1'; // the only data in Hello Message

    /*wrap into packet*/
    Packet *hello_packet;
    hello_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Hello_Msg type Data

    cfgread(filename, "router_id", hello_packet->RouterID); // RouterID, read from cfg file
    snprintf(hello_packet->PacketType, sizeof(hello_packet->PacketType), "%s", "001"); // Hello Packets (001)

    hello_packet->Data = (Hello_Msg) hello_msg; // Data

    /*checksum*/
    snprintf(hello_packet->PacketChecksum, sizeof(hello_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) hello_packet, sizeof(*hello_packet)));

    return hello_packet;
}

/*thread for hello message thread, arg is the client_fd*/
void *hellothread(void *arg){

    int sockfd; // File descriptor and 'read/write' to socket indicator
    sockfd = (int) arg; // Getting sockfd from void arg passed in

    /*get hello interval*/
    char *filename = "../config/lsrp-router.cfg";
    char hello_interval[4];
    cfgread(filename, "hello_interval", hello_interval); // read hello interval from cfg file

    Packet *hello_req, *hello_reply; // MUST use pointer to fit different Packet

    int alive = 1;
    while(alive == 1){
        /* Receive hellos_req from remote side */
        hello_req = (Packet *)malloc(sizeof(Packet));
        Recv(sockfd, hello_req, sizeof(Packet), 0);
        alive = atoi(&(hello_req->Data.Hello)); // set alive from hello_req

        if(alive == 1){
            /* generate hellos_reply reply according to configure file */
            hello_reply = genHelloReq(); // msg to be sent back
            Send(sockfd, hello_reply, sizeof(Packet), 0);

            /* enter into next round after interval */
            sleep(atoi(hello_interval)); // hello_interval, default is 40s
        }
        else{
            perror("hellothread");
        }
    }

    close(sockfd);
    printf("TID:0x%x served request, exiting thread\n", pthread_self());
    pthread_exit(0);
}

