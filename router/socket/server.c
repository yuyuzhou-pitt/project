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
#include <sys/stat.h>
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
#define MAX_QUE_CONN_NM 1024
#define TIMER 3600  //Timeout value in seconds
#define CONTINUE 1 
#define NTHREADS 5

pthread_t threadid[NTHREADS]; // Thread pool
int counter = 0;

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

    int isEndsys = -1;
    int isNeighbor = -1;
    char logmsg[128]; 
    /* There are 5 types of Neighbor Acquisition Packets.
     * Neighbor Acquisition Type:
     * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
     * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
     *                 or Cease_Neighbors_Confirm(101)   */ 
    Packet *neighbor_req, *neighbor_reply; // MUST use pointer to fit different Packet
    
    /* Receive neighbors_req from remote side */
    neighbor_req = (Packet *)malloc(sizeof(Packet));
    Recv(sockfd, neighbor_req, sizeof(Packet), MSG_NOSIGNAL);

    /* Neighbor request packet */
    if(strcmp(neighbor_req->PacketType, "000") == 0){

        /* generate neighbors_reply reply according to configure file */
        neighbor_reply = genNeighborReq(router, threadParam->port); // msg to be sent back
        genNeighborReply(router, neighbor_req, neighbor_reply); // update the Neighbor Acquisition Type
        Send(sockfd, neighbor_reply, sizeof(Packet), MSG_NOSIGNAL);
    
    
        /* if neighbor request confirmed:
         * 1) exchange alive (hello) message in HelloInterval seconds
         * 2) exchange LSA message in UpdateInterval seconds, or every time there is updates
         *     */
    
        snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): reply neighbor acq type: %s\n", \
                                          pthread_self(), neighbor_reply->Data.NeighborAcqType);
        logging(LOGFILE, logmsg);
        /* Be_Neighbors_Confirm: 001 */
        if(strcmp(neighbor_reply->Data.NeighborAcqType, "001") == 0){
            isNeighbor = 1;
        }
    }
    /* Endsystem, data packet */
    else if(strcmp(neighbor_req->PacketType, "100") == 0){
        snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): got data from endsystem %s, enqueue...\n", 
                                          pthread_self(), neighbor_req->RouterID);
        logging(LOGFILE, logmsg);
        int endsysEthx = getEthx(router, neighbor_req->RouterID);
        repackData(router, neighbor_req);
        pthread_mutex_lock(&threadParam->lock_buffer);
        addBufferData(threadParam, neighbor_req);
        pthread_mutex_unlock(&threadParam->lock_buffer);

        /* wait for ACK (110) */
        int sendACK = 0;
        while(sendACK == 0){
            if(threadParam->buffer[endsysEthx].buffsize > 0){
                pthread_mutex_lock(&threadParam->lock_buffer);
                if(strcmp(threadParam->buffer[endsysEthx].packet_q->next->packet->PacketType, "110") == 0 ){
                    sendBufferData(sockfd, &threadParam->buffer[endsysEthx]);
                    sendACK = 1;
                }
                pthread_mutex_unlock(&threadParam->lock_buffer);
            }
        }

        /* each packet comes from end system will start a new socket,
         * close this client and quit this thread according to the endsys logic,
         * or the next packet will never come */
        snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): served one packet from end system, exiting thread\n", pthread_self());
        logging(LOGFILE, logmsg);

    }

    /* continue if communicate with neighbor */
    if(isNeighbor == 1){

        struct timeval *tmpcost, cost, timer; // use high quality timer to calculate the ping cost
        struct timezone tzp;

        /* accept Data from now on */
        while(1){

            /* Receive packet_req from client */
            pthread_mutex_lock(&threadParam->lock_server);

            gettimeofday(&timer, &tzp);

            Packet *packet_req, *packet_reply; // MUST use pointer to fit different Packet
            packet_req = (Packet *)calloc(1, sizeof(Packet));

            Recv(sockfd, packet_req, sizeof(Packet), MSG_NOSIGNAL);
        
            //printf("serverthread(0x%x): packet_req type: %s\n", pthread_self(), packet_req->PacketType);
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
                //pthread_mutex_lock(&lock_server);
                installLSA(threadParam, packet_req); // installs the new LSA in its link state database.
                //genLSAACK(threadParam, packet_req);
                //addBufferACK(threadParam, packet_req, ethx); // ready to send LSA ack
                repackLSA(router, packet_req);
                addBufferFlood(threadParam, packet_req, ethx, timer); // except the ethx from which it received the LSA.
                //pthread_mutex_unlock(&lock_server); // Critical section end
                genGraph(threadParam);
                genRouting(threadParam);
                //min_route(int sid, int did, int *gateway, int *metric);

            }
            /* Data (100) or ACK (110) packet */
            else if(strcmp(packet_req->PacketType, "100") == 0 || strcmp(packet_req->PacketType, "110") == 0){
                //printf("serverthread(0x%x): got data from %s, enqueue...\n", pthread_self(), packet_req->RouterID);
                repackData(router, packet_req);
                addBufferData(threadParam, packet_req);
            }
   
            pthread_mutex_unlock(&threadParam->lock_server); // Critical section end
            usleep(1); // sleep some time or other thread do not have chance to get the lock
        }

    }

    shutdown(sockfd, SHUT_RDWR);
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
    char logmsg[128];

    sin_size=sizeof(client_sockaddr);

    /* Data structure to handle timeout */
    struct timeval before, timer, *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set, test_set;
    int maxfd, nready, client[FD_SETSIZE];

    /* create socket */
    sockfd = Socket(AF_INET,SOCK_STREAM,0);

    /* set parameters for sockaddr_in */
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    Bind(sockfd, server_sockaddr);

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    threadParam->port = port;
    writePort(port, addrstr);
    
    snprintf(logmsg, sizeof(logmsg), "sockserver: Server %s (%s) is setup on port: %d\n", addrstr, hostname, port);
    logging(LOGFILE, logmsg);
    Listen(sockfd, MAX_QUE_CONN_NM);
   
    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)
    int iThread = 0; // Thread iterator

    /* Set up the I/O for the socket, nonblocking */
    maxfd = sockfd;
    int k;
    for(k=0;k<FD_SETSIZE;k++){
        client[k] = -1;
    }
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
                printf("sockserver: errno: %d.\n", errno);
                perror("\nSELECT: unexpected error occured.\n");
                logging(LOGFILE, "\nSELECT: unexpected error occured.\n");

                /* remove bad fd */
                for(k=0;k<FD_SETSIZE;k++){
                    if(client[k] > 0){
                        struct stat tStat;
                        if (-1 == fstat(client[k], &tStat)){
                            printf("fstat %d error:%s", sockfd, strerror(errno));
                            FD_CLR(client[k], &ready_set);
                        }
                    }
                }
                //exit(-1); // do not exit when error happened
                //status=-1;
                //break;
            case 0:
                /* timeout occuired */
                //printf("sockserver: TIMEOUT... %d.\n", errno);
                logging(LOGFILE, "\nsockserver: TIMEOUT...\n");
                //status=-1;
                //break;
            default:
                if (FD_ISSET(sockfd, &ready_set)){
                    snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Listening socket is readable\n", pthread_self());
                    logging(LOGFILE, logmsg);
                    /* wait for connection */
                    client_fd = Accept(sockfd, client_sockaddr, sin_size);
                    for(k=0;k<FD_SETSIZE;k++){
                        if(client[k] < 0){
                            client[k] = client_fd;
                        }
                    }

                    //printf("sockserver: using client_fd: %d\n", client_fd);
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
