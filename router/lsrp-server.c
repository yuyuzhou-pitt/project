/* Multi-thread version socket.
 *  - assign port automatically
 *  - timeout after given time
 *  - accept multi-client connection
 *
 * Steps to build and run:
 * $ make
 * $ ./lsrp-server
*/

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
#include <pthread.h>
#include "socket/libsocket.h"
#include "socket/getaddrinfo.h"
#include "packet/neighbor.h"
#include "config/config.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define MAX_QUE_CONN_NM 5
#define TIMER 1000  //Timeout value in seconds
#define CONTINUE 1 
#define NTHREADS 5

pthread_t threadid[NTHREADS]; // Thread pool
pthread_mutex_t lock;
int counter = 0;

//char *lsrpcfg = "lsrp-router.cfg";
char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)
int port; // local socket port

/*thread for new client connection, arg is the client_fd*/
void *sockthread(void *arg){

    int sockfd; // File descriptor and 'read/write' to socket indicator
    sockfd = (int) arg; // Getting sockfd from void arg passed in

    /* There are 5 types of Neighbor Acquisition Packets.
     * Neighbor Acquisition Type:
     * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
     * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
     *                 or Cease_Neighbors_Confirm(101)   */ 
    Packet *neighbor_req, *neighbor_reply; // MUST use pointer to fit different Packet
    neighbor_reply = genNeighborReq(LSRPCFG, addrstr, port); // msg to be sent back
    
    /* Receive neighbors_req from remote side */
    neighbor_req = (Packet *)malloc(sizeof(*neighbor_reply));
    Recv(sockfd, neighbor_req, sizeof(*neighbor_req), 0);

    /* generate neighbors_reply reply according to configure file */
    genNeighborReply(LSRPCFG, neighbor_req, neighbor_reply); // update the Neighbor Acquisition Type
    Send(sockfd, neighbor_reply, sizeof(*neighbor_reply), 0);

    printf("reply type: %s\n", neighbor_reply->Data.NeighborAcqType);

    /* Critical section */
    pthread_mutex_lock (&lock);
    counter++;
    pthread_mutex_unlock (&lock);
  
    close(sockfd);
    printf("TID:0x%x served request, exiting thread\n", pthread_self());
    pthread_exit(0);

}

int main(void){
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    int sockfd, client_fd, desc_ready;

    sin_size=sizeof(client_sockaddr);

    /* Data structure to handle timeout */
    struct timeval before, timer, *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set, test_set;
    int maxfd, nready, nbytes;

    /* create socket */
    sockfd = Socket(AF_INET,SOCK_STREAM,0);

    /* set parameters for sockaddr_in */
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = INADDR_ANY; //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    Bind(sockfd, server_sockaddr);

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    writePort(port, addrstr);
    
    Listen(sockfd, MAX_QUE_CONN_NM);
   
    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)
    int iThread = 0; // Thread iterator

    /* Set up the I/O for the socket, nonblocking */
    maxfd = sockfd;
    FD_ZERO(&ready_set);
    FD_ZERO(&test_set);
    FD_SET(sockfd, &test_set);
  
    /* Initialize the timeval struct to TIMER seconds */
    timer.tv_sec = TIMER;
    timer.tv_usec = 0;
    tvptr = &timer;

    /* Set up the time out by getting the time of the day from the system */
    gettimeofday(&before, &tzp);

    int status;
    status=CONTINUE;
    while (status==CONTINUE){
        if (iThread == NTHREADS){
            iThread = 0;
        }

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
                if (FD_ISSET(sockfd, &ready_set)){
                    printf("  Listening socket is readable\n");
                    /* wait for connection */
                    client_fd = Accept(sockfd, client_sockaddr, sin_size);
                    FD_SET(client_fd, &test_set);
                    if (client_fd > maxfd) maxfd = client_fd;
                    printf("  Descriptor %d is readable\n", i);

                    pthread_create(&threadid[i++], &attr, &sockthread, (void *) client_fd);
                    sleep(0); // Giving threads some CPU time
                }// end if (FD_ISSET(i, &ready_set))
        }// end switch
    } // end while (status==CONTINUE)
    close(sockfd);
    unlinkPortFile(addrstr);

    return 0;
}
