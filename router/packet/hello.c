#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define HELLO 1
#include "packet.h"

#define LSRPINPACKET "config/lsrp-router.cfg"

Packet *genHelloReq(Router *router){
    /*generate hello message*/

    Hello_Msg hello_msg;
    hello_msg.Hello = 1; // the only data in Hello Message

    /*wrap into packet*/
    Packet *hello_packet;
    hello_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Hello_Msg type Data

    snprintf(hello_packet->RouterID, sizeof(hello_packet->RouterID), "%s", router->router_id);
    snprintf(hello_packet->PacketType, sizeof(hello_packet->PacketType), "%s", "001"); // Hello Packets (001)

    hello_packet->Data = (Hello_Msg) hello_msg; // Data

    /*checksum*/
    snprintf(hello_packet->PacketChecksum, sizeof(hello_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) hello_packet, sizeof(*hello_packet)));

    return hello_packet;
}

/*thread for hello message thread, arg is the client_fd*/
void *helloserver(void *arg){

    int sockfd; // File descriptor and 'read/write' to socket indicator
    sockfd = (int) arg; // Getting sockfd from void arg passed in

    Router *router;
    router = getRouter(LSRPINPACKET); // get router configuration from cfg file

    Packet *hello_req, *hello_reply; // MUST use pointer to fit different Packet

    int alive = 1;
    char logmsg[128];
    while(alive == 1){
        /* Receive hellos_req from remote side */
        hello_req = (Packet *)malloc(sizeof(Packet));
        Recv(sockfd, hello_req, sizeof(Packet), 0);
        alive = hello_req->Data.Hello; // set alive from hello_req

        snprintf(logmsg, sizeof(logmsg), "helloserver: got alive message: %d.\n", alive);
        logging(LOGFILE, logmsg);

        /* generate hellos_reply reply according to configure file */
        hello_reply = genHelloReq(router); // msg to be sent back
        Send(sockfd, hello_reply, sizeof(Packet), 0);

        /* enter into next round after interval */
        snprintf(logmsg, sizeof(logmsg), "helloserver: sleep for %d seconds...\n", router->hello_interval);
        logging(LOGFILE, logmsg);
        sleep(router->hello_interval); // hello_interval, default is 40s
    }

    close(sockfd);
    snprintf(logmsg, sizeof(logmsg), "TID:0x%x served request, exiting thread\n", pthread_self());
    logging(LOGFILE, logmsg);
    pthread_exit(0);
}

/*thread for hello message thread, arg is the client_fd*/
void *helloclient(void *arg){
    int sockfd; // File descriptor and 'read/write' to socket indicator
    sockfd = (int) arg; // Getting sockfd from void arg passed in

    Router *router;
    router = getRouter(LSRPINPACKET); // get router configuration from cfg file

    Packet *hello_req, *hello_reply; // MUST use pointer to fit different Packet

    int alive = 1;
    char logmsg[128];
    while(alive == 1){
        /* generate hellos_reply reply according to configure file */
        hello_req = genHelloReq(router); // msg to be sent back
        Send(sockfd, hello_req, sizeof(Packet), 0);
        snprintf(logmsg, sizeof(logmsg), "helloclient: Hello packet sent.\n");
        logging(LOGFILE, logmsg);

        /* Receive hellos_req from remote side */
        hello_reply = (Packet *)malloc(sizeof(Packet));
        Recv(sockfd, hello_reply, sizeof(Packet), 0);

        alive = hello_reply->Data.Hello; // set alive from hello_req
        snprintf(logmsg, sizeof(logmsg), "helloclient: got alive message: %d.\n", alive);
        logging(LOGFILE, logmsg);

        /* enter into next round after interval */
        snprintf(logmsg, sizeof(logmsg), "helloclient: sleep for %d seconds...\n", router->hello_interval);
        logging(LOGFILE, logmsg);
        sleep(router->hello_interval); // hello_interval, default is 40s
    }

    close(sockfd);
    snprintf(logmsg, sizeof(logmsg), "TID:0x%x served request, exiting thread\n", pthread_self());
    logging(LOGFILE, logmsg);
    pthread_exit(0);
}
