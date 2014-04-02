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
    char buff[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    struct hostent *host;
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

    /*connect to server*/
    Connect(sockfd,sockaddr,sizeof(sockaddr));

    /*send message*/
    memset(buff, 0, sizeof(buff));
    strcpy(buff,argv[2]);
    sendbytes = Send(sockfd,buff,sizeof(buff), 0);

    /*receive feedback*/
    memset(buf,0,sizeof(buf));
    recvbytes = Recv(sockfd,buf,sizeof(buf), 0);

    //sleep(3600);
    close(sockfd);
    exit(0);
}
