/* Multi-thread version socket.
 *  - assign port automatically
 *  - timeout after given time
 *  - accept multi-client connection
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
#include "../packet/hello.h"
#include "../packet/libqueue.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define MAX_QUE_CONN_NM 5
#define TIMER 1000  //Timeout value in seconds
#define CONTINUE 1 
#define NTHREADS 5
//#define LSRPCFG "config/lsrp-router.cfg"

pthread_t threadid[NTHREADS]; // Thread pool
pthread_mutex_t lockserver = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

//char *lsrpcfg = "lsrp-router.cfg";
char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)
int port; // local socket port


/*thread for new client connection, arg is the client_fd*/
void *serverthread(void *arg){

    /* Getting sockfd and router from void arg passed in */
    ThreadParam *threadParam;
    threadParam = (ThreadParam *) arg;
    int sockfd = threadParam->sockfd;
    Router *router;
    router = threadParam->router;

    /* There are 5 types of Neighbor Acquisition Packets.
     * Neighbor Acquisition Type:
     * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
     * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
     *                 or Cease_Neighbors_Confirm(101)   */ 
    Packet *neighbor_req, *neighbor_reply; // MUST use pointer to fit different Packet
    
    /* Receive neighbors_req from remote side */
    neighbor_req = (Packet *)malloc(sizeof(Packet));
    Recv(sockfd, neighbor_req, sizeof(Packet), MSG_NOSIGNAL);

    /* generate neighbors_reply reply according to configure file */
    neighbor_reply = genNeighborReq(router, threadParam->port); // msg to be sent back
    genNeighborReply(router, neighbor_req, neighbor_reply); // update the Neighbor Acquisition Type
    Send(sockfd, neighbor_reply, sizeof(Packet), MSG_NOSIGNAL);


    /* if neighbor request confirmed:
     * 1) exchange alive (hello) message in HelloInterval seconds
     * 2) exchange LSA message in UpdateInterval seconds, or every time there is updates
     *     */

    char logmsg[128]; 
    snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): reply neighbor acq type: %s\n", \
                                      pthread_self(), neighbor_reply->Data.NeighborAcqType);
    logging(LOGFILE, logmsg);

    if(strcmp(neighbor_reply->Data.NeighborAcqType, "001") == 0){

        /* internal buffer, do not exchange to other routers */
        //Packet_Buff *neighbor_buff, *hello_buff, *lsa_buff, *ping_buff;
        
        /* use a thread to keep alive */
        //pthread_t hellothreadid;
        //pthread_create(&hellothreadid, NULL, &helloserver, (void *) sockfd);

        /* use another thread for LSA */
        //pthread_t LSAthreadid;
        //pthread_create(&LSAthreadid, NULL, &LSAserver, (void *) sockfd);

        struct timeval *tmpcost, cost, timer; // use high quality timer to calculate the ping cost
        struct timezone tzp;

        /* accept Data from now on */
        while(1){

            /* Receive packet_req from client */
            pthread_mutex_lock(&lockserver);

            gettimeofday(&timer, &tzp);

            Packet *packet_req, *packet_reply; // MUST use pointer to fit different Packet
            packet_req = (Packet *)calloc(1, sizeof(Packet));

            Recv(sockfd, packet_req, sizeof(Packet), MSG_NOSIGNAL);
        
            //printf("serverthread(0x%x): got packet from %s with PacketType = %s \n", pthread_self(), packet_req->RouterID, packet_req->PacketType);

            int ethx = getEthx(router, packet_req->RouterID);
            /* Neighbor packet */
            if(strcmp(packet_req->PacketType, "000") == 0){
                //sendNeighborReply(sockfd, packet_req, router, threadParam->port);
            }
            /* Hello packet */
            else if(strcmp(packet_req->PacketType, "001") == 0){
                //sendHello(sockfd, router, threadParam->port);
                //printf("serverthread: got hello packet from %s\n", packet_req->RouterID);
            }
            /* Ping packet */
            else if(strcmp(packet_req->PacketType, "011") == 0 ){
                /* calculate link cost */
                char remote_eth_id[32];
                memset(remote_eth_id, 0, sizeof(remote_eth_id));
                //printf("serverthread: got ping packet from %s\n", packet_req->RouterID);
                tmpcost = calCost(packet_req, router->ping_alpha, &cost, timer, remote_eth_id);
                //cost.tv_sec = tmpcost->tv_sec;
                //cost.tv_usec = tmpcost->tv_usec;

                //printf("serverthread(0x%x): cost is: %d:%d\n", cost.tv_sec, cost.tv_usec);
                router->ethx[ethx].link_cost = cost; // update router for LSA packet
                strncpy(router->ethx[ethx].direct_link_eth_id, remote_eth_id, strlen(remote_eth_id)); // update router for LSA packet
            }
            /* LSA packet */
            else if(strcmp(packet_req->PacketType, "010") == 0){
                /* use LSA to fill the LS Database */
                //printf("serverthread: got LSA packet from %s\n", packet_req->RouterID);
                //pthread_mutex_lock(&lockserver);
                installLSA(threadParam, packet_req); // installs the new LSA in its link state database.
                //genLSAACK(threadParam, packet_req);
                //addBufferACK(threadParam, packet_req, ethx); // ready to send LSA ack
                //repackLSA(router, packet_req);
                addBufferFlood(threadParam, packet_req, ethx); // except the ethx from which it received the LSA.
                //pthread_mutex_unlock(&lockserver); // Critical section end
                genGraph(threadParam);
                genRouting(threadParam);
                //min_route(int sid, int did, int *gateway, int *metric);

            }
            /* Data packet */
            else if(strcmp(packet_req->PacketType, "100") == 0){
                // data, addBuff();
                //Thans_Data trans_data = (Trans_Data)packet->Data;
                //getEthx(router, trans_data.des_ip); // calculate the out interface according to the routing table
                addBufferData(threadParam, packet_req);
            }
   
            pthread_mutex_unlock(&lockserver); // Critical section end
            usleep(1); // sleep some time or other thread do not have chance to get the lock
        }

    }

    //sleep(3600);

    /* Critical section */
    pthread_mutex_lock(&lockserver);
    counter++;
    pthread_mutex_unlock(&lockserver);
  
    close(sockfd);
    snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): served request, exiting thread\n", pthread_self());
    logging(LOGFILE, logmsg);

    pthread_exit(0);

}

/* thread for lsrp-server main */
void *sockserver(void *arg){
    ThreadParam *threadParam;
    threadParam = (ThreadParam *)arg;

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
    threadParam->port = port;
    writePort(port, addrstr);
    
    Listen(sockfd, MAX_QUE_CONN_NM);
   
    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
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
    char logmsg[128];
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
                logging(LOGFILE, "\nTIMEOUT...\n");
                status=-1;
                break;
            default:
                if (FD_ISSET(sockfd, &ready_set)){
                    snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Listening socket is readable\n", pthread_self());
                    logging(LOGFILE, logmsg);
                    /* wait for connection */
                    client_fd = Accept(sockfd, client_sockaddr, sin_size);
                    FD_SET(client_fd, &test_set);
                    if (client_fd > maxfd) maxfd = client_fd;
                    snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Descriptor %d is readable\n",  pthread_self(), client_fd);
                    logging(LOGFILE, logmsg);

                    threadParam->sockfd = client_fd;
                    pthread_create(&threadid[i++], &attr, &serverthread, (void *)threadParam);
                    //sleep(1); // Giving threads some CPU time
                }// end if (FD_ISSET(i, &ready_set))
        }// end switch
    } // end while (status==CONTINUE)
    close(sockfd);
    unlinkPortFile(addrstr);

    return 0;
}
