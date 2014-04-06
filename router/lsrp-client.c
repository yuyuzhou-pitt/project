/* Demo of socket client.
 * Compile and run:
 * $ gcc -o client client.c libsocket.c
 * $ ./client <server-ip> <server-port> "<message>"
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "socket/libsocket.h"
#include "packet/neighbor.h"
#include "config/config.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

int main(int argc, char *argv[]){
    int sockfd,sendbytes,recvbytes;
    char buff[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    struct hostent *host;
    struct sockaddr_in sockaddr;

    if(argc < 2){
        fprintf(stderr,"USAGE: ./client Hostname(or ip address) \n");
        exit(1);
    }

    /*address resolution*/
    if ((host = gethostbyname(argv[1])) == NULL){
        perror("gethostbyname");
        exit(1);
    }

    /*read from target host file*/
    char portstr[6]; 
    getPort(portstr, argv[1]);

    /*create socket*/
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(portstr));
    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);         

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h

    /*connect to server*/
    Connect(sockfd,sockaddr,sizeof(sockaddr));

    /* There are 5 types of Neighbor Acquisition Packets.
     * Neighbor Acquisition Type:
     * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
     * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
     *                 or Cease_Neighbors_Confirm(101)   */
    Packet *neighbor_req, *neighbor_reply;

    /* generate neighbors_req according to configure file */
    neighbor_req = genNeighborReq(LSRPCFG, addrstr, atoi(portstr)); // msg to be sent out
    Send(sockfd, neighbor_req, sizeof(*neighbor_req), 0);

    neighbor_reply = (Packet *)malloc(sizeof(*neighbor_req));
    /* Receive neighbors_reply from remote side */
    Recv(sockfd, neighbor_reply, sizeof(*neighbor_reply), 0);

    /*send message*/
    /*memset(buff, 0, sizeof(buff));
    strcpy(buff,argv[2]);
    sendbytes = Send(sockfd,buff,sizeof(buff), 0);
    */

    /*receive feedback*/
    /*
    memset(buf,0,sizeof(buf));
    recvbytes = Recv(sockfd,buf,sizeof(buf), 0);
    */

    //sleep(3600);
    close(sockfd);
    exit(0);
}