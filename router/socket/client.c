/* Demo of socket client.
 * 1) client starts and run into busy wait
 * 2) check the server file 
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "libsocket.h"
#include "../packet/neighbor.h"
#include "../packet/hello.h"
#include "../packet/ping.h"
#include "../packet/libqueue.h"
#include "../packet/lsa.h"
#include "../config/config.h"
#include "../config/liblog.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024
//#define LSRPCFG "config/lsrp-router.cfg"
#define READ_PORT_INTERVAL 10 // busy wait interval to read port files
//#define ENDSYS_PORTFILE "../endsys/"

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

pthread_mutex_t lock, lock_send, lock_endsys, lock_buffer;

/* thread for lsrp-client */
void *sockclient(void *arg){
//int main(void){
    ThreadParam *threadParam;
    threadParam = (ThreadParam *) arg;

    int clientfd,sendbytes,recvbytes;
    struct hostent *host;
    struct sockaddr_in sockaddr;

    Router *router;
    router = threadParam->router; // get the router cfg
    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
   
    /* try to get server hostname and port from the host files (.<servername>) */
    char remote_server[32];
    char portstr[6]; // to store the port
    int endsysEthx = -1; // set as iEthx if it's endsys
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): busy wait for new server starting up...\n", pthread_self());
    logging(LOGFILE, logmsg);
    while(1){
        int iEthx;
        memset(portstr, 0, sizeof(portstr)); 
        pthread_mutex_lock(&lock); // Critical section to read port files
        /* Steps:
         * 1) go through all the direct_link_addr in cfg file
         * 2) if port file do NOT exists, busy wait
         * 3) if port file exists and occupied, skip it
         * 4) if port file exists and NOT occupied, use it
         * 5) update client ip into the port file
         */

        /* 1) go through all the direct_link_addr in cfg file */
        for(iEthx=0;iEthx < router->num_of_interface; iEthx++){
            if(strcmp(router->ethx[iEthx].direct_link_addr, addrstr) != 0){ // skip the local server file
                char templine[128];
                int iget;
                /* 2) if port file do NOT exists, busy wait */
                if((iget = getPort(templine, sizeof(templine), router->ethx[iEthx].direct_link_addr)) < 0 ){ // read the port file
                    continue; // if file does not exists, continue
                }
                /* 3) if port file exists and occupied, check if I'm in the list or not
                 */
                int portFound = 0;
                char tempport[6];
                if(strlen(templine) > 5){
                    char tempsplit[128];
                    strcpy(tempsplit, templine);
                    char *split1, *split2, *strsplit = &tempsplit;
                    int inList = 0;
                    while(strlen(strsplit) > 0){
                        split1 = strtok_r(strsplit, ":", &split2);
                        if(strlen(split1) == 5){
                            snprintf(tempport, sizeof(tempport), "%s", split1);
                        }
                        else if(strcmp(split1, addrstr) == 0){
                            inList = 1; // in the list
                            break; // break while(strlen(strsplit) > 0)
                        }
                        strsplit = split2;
                    }

                    /* not in the list, then prepare to add me in */
                    if(inList == 0){
                        snprintf(portstr, sizeof(portstr), "%s", tempport);
                        portFound = 1;
                    }
                    //snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): %s:%s is occupied, try next one.\n", \
                    //                                  pthread_self(), router->ethx[iEthx].direct_link_addr, tempport);
                    //logging(LOGFILE, logmsg);
                }
                /* 4) if port file exists and NOT occupied, use it */
                else if(strlen(templine) == 5){
                    snprintf(portstr, sizeof(portstr), "%s", templine);
                    portFound = 1;
                }

                if(portFound == 1){
                    if(router->ethx[iEthx].direct_link_type == 1){
                        endsysEthx = iEthx;
                    }
                    snprintf(remote_server, sizeof(remote_server), "%s", router->ethx[iEthx].direct_link_addr);
                    router->ethx[iEthx].direct_link_port = atoi(portstr);
                    if((host = gethostbyname(router->ethx[iEthx].direct_link_addr)) == NULL ) { // got the remote server
                        perror("gethostbyname");
                        exit(-1);
                    };
                    /* 5) update client ip into the port file, in the format:
                     *    34322:10.0.0.1:10.0.0.2
                     */
                    strcat(templine, ":");
                    strcat(templine, addrstr);
                    markPort(router->ethx[iEthx].direct_link_addr, templine); // mark that port has been used in the port file
                    snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): client %s use port %s:%s now.\n", pthread_self(), \
                                                      addrstr, router->ethx[iEthx].direct_link_addr, portstr);
                    logging(LOGFILE, logmsg);

                    /* 6) connect to remote server */
                    /*create socket*/
                    clientfd = Socket(AF_INET, SOCK_STREAM, 0);
                
                    /*parameters for sockaddr_in*/
                    sockaddr.sin_family = AF_INET;
                    sockaddr.sin_port = htons(atoi(portstr));
                    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
                    bzero(&(sockaddr.sin_zero), 8);         
                
                    /*connect to server*/
                    Connect(clientfd,sockaddr,sizeof(sockaddr));

                    break; //end the for loop
                }
            }
        }
        pthread_mutex_unlock(&lock); // Critical section end
    
        sleep(READ_PORT_INTERVAL);

        if(strlen(portstr) == 5){
            break; // end the while loop
        }
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): No available port found, make sure all servers in direct_link_addr started.\n", pthread_self());
        logging(LOGFILE, logmsg);
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): wait %d seconds to try again..\n", pthread_self(), READ_PORT_INTERVAL);
        logging(LOGFILE, logmsg);
    }

    //pthread_mutex_lock(&lock_endsys);
    /* deal with end sys */
    if(endsysEthx != -1 ){
        while(1){
            /* read data from buffer */
            printf("i'm handling data from endsys..\n");
            pthread_mutex_lock(&lock_buffer);
            if(threadParam->buffer[endsysEthx].buffsize > 0){
                sendBufferData(clientfd, &threadParam->buffer[endsysEthx]);

            }
            pthread_mutex_unlock(&lock_buffer);
            usleep(1); //sleep some time for lock relase
        }
    }
    else{

        /* there are 5 types of Neighbor Acquisition Packets.
         * neighbor Acquisition Type:
         * neighbor_req: Be_Neighbors_Request(000) or Cease_Neighbors_Request(100)
         * neighbor_reply: Be_Neighbors_Confirm(101), Be_Neighbors_Refuse(111),
         *                 or Cease_Neighbors_Confirm(101)   */
        Packet *neighbor_req, *neighbor_reply;
    
        /* generate neighbors_req according to configure file */
        neighbor_req = genNeighborReq(router, atoi(portstr)); // msg to be sent out
        send(clientfd, neighbor_req, sizeof(Packet), MSG_NOSIGNAL);
    
        neighbor_reply = (Packet *)malloc(sizeof(Packet));
        /* Receive neighbors_reply from remote side */
        Recv(clientfd, neighbor_reply, sizeof(Packet), MSG_NOSIGNAL);
    
        if(strcmp(neighbor_reply->Data.NeighborAcqType, "001") == 0){
            //pthread_t hellothreadid;
            //pthread_create(&hellothreadid, NULL, &helloclient, (void *) clientfd);
    
            int hello_interval = router->hello_interval;
            int ping_interval = router->ping_interval;
            int ls_updated_interval = router->ls_updated_interval;
    
            int hello_sent = 0, ping_sent = 0, lsa_sent = 0;
            time_t now1, now2, now3;
            int ls_sequence_number = 0;
    
            struct timeval timer; // use high quality timer to calculate the ping cost
            struct timezone tzp;
    
            int ethx = getEthx(router, remote_server);
            Packet *packet_req, *packet_reply; // MUST use pointer to fit different Packet
    
            //int clientfd = router->clientfd;
            while(1){
    
                gettimeofday(&timer, &tzp);
                time_t now = timer.tv_sec;
    
                pthread_mutex_lock(&lock_send); // Critical section to read port files
                /* hello message */
                if(now % hello_interval ==0){
                    if(hello_sent == 0){
                        now1 = now;
                        hello_sent = 1;
        
                        //printf("hello_interval=%d\n", hello_interval);
    
                        //pthread_mutex_lock(&lock_send); // Critical section to read port files
                        sendHello(clientfd, router, atoi(portstr));
                        //pthread_mutex_unlock(&lock_send); // Critical section end
    
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): Hello packet sent.\n", pthread_self());
                        logging(LOGFILE, logmsg);
                    }
                    if(now != now1){
                        hello_sent = 0;
                    }
                }
                /* ping message */
                if(now % ping_interval ==0){
                    if(ping_sent == 0){
                        now1 = now;
                        ping_sent = 1;
        
                        //printf("ping_interval=%d\n", ping_interval);
                        //pthread_mutex_lock(&lock_send); // Critical section to read port files
                        sendPing(clientfd, router, timer, ethx);
                        //pthread_mutex_unlock(&lock_send); // Critical section end
    
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): Ping packet sent.\n", pthread_self());
                        logging(LOGFILE, logmsg);
                    }
                    if(now != now1){
                        ping_sent = 0;
                    }
                }
                /* lsa message */
                if(now % ls_updated_interval ==0){
                    if(lsa_sent == 0){
                        now2 = now;
                        lsa_sent = 1;
                        ls_sequence_number++;
    
                        /* TODO: check acknowledgment, keep sending if no ack */
                        //pthread_mutex_lock(&lock_send); // Critical section to read port files
                        //printf("sockclient(0x%x): send LSA packet from %s to %s\n", pthread_self(), addrstr, remote_server);
                        sendNewLSA(clientfd, threadParam, ls_sequence_number, timer);
                        //pthread_mutex_unlock(&lock_send); // Critical section end
    
                        //sendNewLSA(clientfd, router, ls_sequence_number, timer, atoi(portstr), remote_server);
                        //printf("ls_updated_interval=%d\n", ls_updated_interval);
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): LSA packet sent.\n", pthread_self());
                        logging(LOGFILE, logmsg);
                    }
                    if(now != now2){
                        lsa_sent = 0;
                    }
                }
                pthread_mutex_unlock(&lock_send); // Critical section end
    
                /* read data from buffer */
                pthread_mutex_lock(&lock_buffer);
                //printf("sockclient(0x%x): threadParam->buffer[%d].buffsize: %d\n", pthread_self(), ethx, threadParam->buffer[ethx].buffsize);
                if(threadParam->buffer[ethx].buffsize > 0){
                    /* p *threadParam
                    $14 = {sockfd = 8, port = 46976, router = 0x608250, ls_db_size = 1, ls_db = {{Link_ID = "a", '\000' <repeats 30 times>, 
                    des_router_id = '\000' <repeats 31 times>, des_port_id = 12589, src_router_id = "10.0.0.1", '\000' <repeats 17 times>, 
                    src_port_id = 12589, Availability = 0, Link_Cost = {tv_sec = 52992208, tv_usec = 0}, LS_Age = 1397489400}, {
                    Link_ID = '\000' <repeats 31 times>, des_router_id = '\000' <repeats 31 times>, des_port_id = 0, 
                    src_router_id = '\000' <repeats 31 times>, src_port_id = 0, Availability = 0, Link_Cost = {tv_sec = 0, tv_usec = 0}, 
                    LS_Age = 0} <repeats 1023 times>}, buffer = {{buffsize = 0, packet_q = 0x60c3a0}, {buffsize = 0, packet_q = 0x60c3e0}, 
                    {buffsize = 0,   packet_q = 0x0} <repeats 126 times>}}
                    */
    
                    /* packet_q->next is the fist item in the queue
                    (gdb) p *threadParam->buffer[0].packet_q
                    $17 = {packet = 0x0, next = 0x60f000}
                    (gdb) p packet
                    $18 = (Packet *) 0x60f600
                    (gdb) p *threadParam->buffer[0].packet_q->next
                    $20 = {packet = 0x60f600, next = 0x60c3c0}
                    */
    
    
                    /* Neighbor packet */
                    if(strcmp(threadParam->buffer[ethx].packet_q->next->packet->PacketType, "000") == 0){
                        // not possible
                    }
                    /* Hello packet */
                    else if(strcmp(threadParam->buffer[ethx].packet_q->next->packet->PacketType, "001") == 0){
                        // do nothing
                    }
                    /* Ping packet */
                    else if(strcmp(threadParam->buffer[ethx].packet_q->next->packet->PacketType, "011") == 0){
                        // do nothing
                    }
                    /* LSA packet */
                    else if(strcmp(threadParam->buffer[ethx].packet_q->next->packet->PacketType, "010") == 0){
                        //printf("sockclient(0x%x): send original LSA %s packet to %s\n", pthread_self(), threadParam->buffer[ethx].packet_q->next->packet->RouterID, remote_server);
                        sendBufferLSA(clientfd, &threadParam->buffer[ethx]);
                    }
                    /* Data packet */
                    else if(strcmp(threadParam->buffer[ethx].packet_q->next->packet->PacketType, "100") == 0){
                        sendBufferData(clientfd, &threadParam->buffer[ethx]);
                    }
    
                }
                pthread_mutex_unlock(&lock_buffer);
                usleep(1); //sleep some time for lock relase
     
                /* Receive packet_req from server */
                //packet_reply = (Packet *)malloc(sizeof(Packet));
                //Recv(clientfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);
    
                /* do nothing after receive packet from server ?? */
            } //endof while(1)
    
        } // endof if(strcmp(neighbor_reply
    } // endof if endsysEthx
    //pthread_mutex_lock(&lock_endsys);

    //sleep(3600);
    close(clientfd);
    pthread_exit(0);
    //exit(0);
}
