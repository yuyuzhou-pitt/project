/* Demo of socket client.
 * 1) client starts and run into busy wait
 * 2) check the server file 
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
#include "../packet/neighbor.h"
#include "../packet/hello.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024
//#define LSRPCFG "config/lsrp-router.cfg"
#define READ_PORT_INTERVAL 10 // busy wait interval to read port files

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

pthread_mutex_t lock;

/* thread for lsrp-client */
void *sockclient(void *arg){
//int main(void){
    int sockfd,sendbytes,recvbytes;
    struct hostent *host;
    struct sockaddr_in sockaddr;

    Router *router;
    router = (Router *)arg; // get the router cfg
    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
   
    /* try to get server hostname and port from the host files (.<servername>) */
    char portstr[6]; // to store the port
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): busy wait for new servers starting up...\n", pthread_self());
    logging(LOGFILE, logmsg);
    while(1){
        int iEthx;
        memset(portstr, 0, sizeof(portstr)); 
        pthread_mutex_lock (&lock); // Critical section to read port files
        for(iEthx=0;iEthx < router->num_of_interface; iEthx++){
            char *templine;
            char *tempport;
            char *tempserver;
            int iget;
            if((iget = getPort(templine, router->ethx[iEthx].direct_link_addr)) < 0 ){ // read the port file
                continue; // if file does not exists, continue
            }
            if(strlen(templine) > 6){
                tempport = strtok_r(templine, "=", &tempserver);
                snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): port %s on server %s is occupied, please try a new one.\n", \
                                                  pthread_self(), tempport, router->ethx[iEthx].direct_link_addr);
                logging(LOGFILE, logmsg);
            }
            else if(strlen(templine) == 6){
                snprintf(portstr, sizeof(portstr), "%s", templine); // got the remote port
                if((host = gethostbyname(router->ethx[iEthx].direct_link_addr)) == NULL ) { // got the remote server
                    perror("gethostbyname");
                    exit(-1);
                };
                markPort(router->ethx[iEthx].direct_link_addr, portstr, hostname); // mark that port has been used in the port file
                snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): use port %s on server %s.\n", pthread_self(), templine, \
                                                  router->ethx[iEthx].direct_link_addr);
                logging(LOGFILE, logmsg);
                break; //end the for loop
            }
        }
        pthread_mutex_unlock (&lock); // Critical section end
    
        if(strlen(portstr) == 6){
            break; // end the while loop
        }
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): Fail to get the port, make sure all servers in direct_link_addr started.\n", pthread_self());
        logging(LOGFILE, logmsg);
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): wait %d seconds to try again..\n", pthread_self(), READ_PORT_INTERVAL);
        logging(LOGFILE, logmsg);
        sleep(READ_PORT_INTERVAL);
    }

    /*create socket*/
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(portstr));
    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);         


    /*connect to server*/
    Connect(sockfd,sockaddr,sizeof(sockaddr));

    /* There are 5 types of Neighbor Acquisition Packets.
     * Neighbor Acquisition Type:
     * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
     * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
     *                 or Cease_Neighbors_Confirm(101)   */
    Packet *neighbor_req, *neighbor_reply;

    /* generate neighbors_req according to configure file */
    neighbor_req = genNeighborReq(router, addrstr, atoi(portstr)); // msg to be sent out
    Send(sockfd, neighbor_req, sizeof(Packet), 0);

    neighbor_reply = (Packet *)malloc(sizeof(Packet));
    /* Receive neighbors_reply from remote side */
    Recv(sockfd, neighbor_reply, sizeof(Packet), 0);

    if(strcmp(neighbor_reply->Data.NeighborAcqType, "001") == 0){
        pthread_t hellothreadid;
        pthread_create(&hellothreadid, NULL, &helloclient, (void *) sockfd);
    }

    sleep(3600);
    close(sockfd);
    exit(0);
}
