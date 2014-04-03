/* Demo of Multi-thread version socket.
 *  - assign port automatically
 *  - timeout after given time
 *  - accept multi-client connection
 *
 * Compile and run:
 * $ gcc -g -pthread -o server server.c libsocket.c
 * ./server
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
#include "libsocket.h"
#include "getaddrinfo.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define MAX_QUE_CONN_NM   5
#define TIMER  1000  //Timeout value in seconds
#define CONTINUE     1 
#define NTHREADS 5
#define MYIPADDR 136.142.227.15

pthread_t threadid[NTHREADS]; // Thread pool
pthread_mutex_t lock;
int counter = 0;

/*thread for new client connection, arg is the client_fd*/
void *sockthread(void *arg){

    int sockfd; // File descriptor and 'read/write' to socket indicator
    sockfd = (int) arg; // Getting sockfd from void arg passed in

    char buff[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    //neighbor_acq_msg *neighbor_req, *neighbor_cfm, *neighbor_rfs, *cease_req, *cease_cfm;
    
    /* Receive from the remote side */
    memset(buff, 0, sizeof(buff));
    Recv(sockfd,buff,sizeof(buff),0);
    //Recv(sockfd,neighbor_req,sizeof(neighbor_req),0);

    //neighbor_cfm = genCfm(neighbor_req);

    /* confiem the request */
    //if(neighbor_cfm.NeighborAcqType == 1)
    /* Send to the remote side */
    memset(buf , 0, sizeof(buf));
    strcpy(buf,"return to client");
    Send(sockfd,buf, sizeof(buf),0);
  
    /* Critical section */
    printf("Requesting mutex lock...\n");
    pthread_mutex_lock (&lock);
    printf("Current counter value: %d, upping by 1...\n", counter);
    counter++;
    pthread_mutex_unlock (&lock);
    printf("Done! Mutex unlocked again, new counter value: %d\n", counter);
  
    close(sockfd);
    printf("TID:0x%x served request, exiting thread\n", pthread_self());
    pthread_exit(0);

}

int main(){
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    int sockfd, client_fd, desc_ready;

    sin_size=sizeof(client_sockaddr);

    /*Data structure to handle timeout*/
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

    int port;
    char str[6], hostname[1024], addrstr[100];
    getaddr(hostname, addrstr); //get hostname and ip
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    sprintf(str, "%d", port); // int to str
    writePort(str, addrstr);
    
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
    exit(0);
}