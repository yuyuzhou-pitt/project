#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include "libsocket.h"

#define PORT            0 //0 means assign a port randomly 4321
#define BUFFER_SIZE        1024
#define MAX_QUE_CONN_NM   5
#define TIMER  30  /*Timeout value*/ 
#define CONTINUE     1 

int main(){
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    sin_size=sizeof(client_sockaddr);
    int sockfd, client_fd, desc_ready;
    int status;
    char buf[BUFFER_SIZE];
    my buff;
    buff.num=10;
    strcpy(buff.group,"bat001");

    /*Data structure to handle timeout*/
    struct timeval before;
    struct timeval timer;
    struct timeval *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set;
    fd_set test_set;
    int maxfd;
    int nready;
    int nbytes;

    sockfd = Socket(AF_INET,SOCK_STREAM,0);

    /*set parameters for sockaddr_in*/
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = INADDR_ANY; //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

    //Ioctl(sockfd, FIONBIO, i); // Set socket to be non-blocking
    Bind(sockfd, server_sockaddr);
    Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    Listen(sockfd, MAX_QUE_CONN_NM);
   
    /*Set up the I/O for the socket, nonblocking*/
    maxfd = sockfd;
    FD_ZERO(&ready_set);
    FD_ZERO(&test_set);
    FD_SET(sockfd, &test_set);
  
    timer.tv_sec = TIMER; //Initialize the timeval struct to TIMER seconds.
    timer.tv_usec = 0;
    tvptr = &timer;

    /* Set up the time out by getting the time of the day from the system */
    gettimeofday(&before, &tzp);
    status=CONTINUE;
    while (status==CONTINUE){
        memcpy(&ready_set, &test_set, sizeof(test_set));
        nready = select(maxfd+1, &ready_set, NULL, NULL, tvptr);
        switch(nready){
            case -1:
                perror("\nSELECT: unexpected error occured.\n" );
                exit(-1);
                status=-1;
                break;
            case 0:
                /* timeout occuired */
                printf("\nTIMEOUT...\n");
                status=-1;
                break;
            default:
                desc_ready = nready;
                for (i=0; i <= maxfd  &&  desc_ready > 0; ++i){
                    if (FD_ISSET(i, &ready_set)){
                        desc_ready -= 1;

                        if (i == sockfd){
                            printf("  Listening socket is readable\n");
                            //do{
                                /* wait for connection */
                                client_fd = Accept(sockfd, client_sockaddr, sin_size);
                                FD_SET(client_fd, &test_set);
                                if (client_fd > maxfd) maxfd = client_fd;
                            //}while(client_fd != -1);

                        }
                        else{
                            printf("  Descriptor %d is readable\n", i);
         
                            //do{
                                my buff;
				/* Receive from the remote side */
				memset(buf , 0, sizeof(buf));
                                Recvfrom(client_fd,buff.group,sizeof(buff.group),0,client_sockaddr, sin_size);
                                /* Send to the remote side */
                                memset(buf , 0, sizeof(buf));
                                strcpy(buf,"return to client");
                                Sendto(client_fd,buf, strlen(buf),0, client_sockaddr, sin_size);
                                status=-1;
                            //}while(1);
                        }//end else
                    }// end if (FD_ISSET(i, &ready_set))
                }// end for
        }// end switch
    } // end while (status==CONTINUE)
    close(sockfd);
    exit(0);
}
