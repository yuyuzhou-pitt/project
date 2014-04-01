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

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define MAX_QUE_CONN_NM   5
#define TIMER  3000  //Timeout value
#define CONTINUE     1 
#define NTHREADS 5

pthread_t threadid[NTHREADS]; // Thread pool
pthread_mutex_t lock;
int counter = 0;

void *sockthread(void *arg){

    int sockfd; // File descriptor and 'read/write' to socket indicator
    char buff[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    sockfd = (int) arg; // Getting sockfd from void arg passed in
  
    printf("here we are.\n");
    /* Receive from the remote side */
    memset(buff, 0, sizeof(buff));
    Recv(sockfd,buff,sizeof(buff),0);
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

    pthread_attr_t attr; // Thread attribute
    int iThread = 0; // Thread iterator

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
   
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)
                                                               // waiting on each other

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
    exit(0);
}
