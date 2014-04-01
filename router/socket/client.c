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
#include "libsocket.h"
#define PORT   0 //4321
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    int sockfd,sendbytes,recvbytes;
    //char msg[BUFFER_SIZE];
    my buff;
    char buf[BUFFER_SIZE];
    struct hostent *host;
    //struct serv_addr_in serv_addr;
    struct sockaddr_in sockaddr;
    if(argc < 3){
        fprintf(stderr,"USAGE: ./client Hostname(or ip address) Text\n");
        exit(1);
    }
    /*address resolution*/
    if ((host = gethostbyname(argv[1])) == NULL){
        perror("gethostbyname");
        exit(1);
    }
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s", argv[3]);
    /*create socket*/
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(argv[2]));
    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);         
    /*connect to server*/
    Connect(sockfd,sockaddr,sizeof(sockaddr));
    strcpy(buff.group,argv[3]);
    //strcpy(buff.group,"bat001");
    /*send message, use sendto for udp connection*/
    sendbytes = Send(sockfd,buff.group,sizeof(buff.group), 0);
    memset(buf,0,sizeof(buf));
    /*receive feedback*/
    recvbytes = Recv(sockfd,buf,sizeof(buf), 0);
    //printf("Received a message : %s\n",buf);
    close(sockfd);
    exit(0);
}
