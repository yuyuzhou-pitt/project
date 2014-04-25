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
#define READ_PORT_INTERVAL 20 // busy wait interval to read port files

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

/* thread for lsrp-client */
void *sockclient(void *arg){
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
        pthread_mutex_lock(&threadParam->lock_port); // Critical section to read port files
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
                    char *split1, *split2, *strsplit;
                    strsplit = tempsplit;
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
        pthread_mutex_unlock(&threadParam->lock_port); // Critical section end
    
        sleep(READ_PORT_INTERVAL);

        if(strlen(portstr) == 5){
            break; // end the while loop
        }
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): No available port found, make sure all servers in direct_link_addr started.\n", pthread_self());
        logging(LOGFILE, logmsg);
        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): wait %d seconds to try again..\n", pthread_self(), READ_PORT_INTERVAL);
        logging(LOGFILE, logmsg);
    }

    /* deal with end sys */
    if(endsysEthx != -1 ){
        int needACK;
        while(1){
            needACK = 0;
            /* read data from buffer */
            pthread_mutex_lock(&threadParam->data_buffer[endsysEthx].lock_buffer);
            if(threadParam->data_buffer[endsysEthx].buffsize > 0){
                //printf("sockclient(0x%x): threadParam->data_buffer[%d].buffsize: %d, type of the first item: %s\n", 
                //    pthread_self(), endsysEthx, threadParam->data_buffer[endsysEthx].buffsize,
                //    threadParam->data_buffer[endsysEthx].packet_q->next->packet->PacketType);

                /* out going buffer devided by interface, maybe there are other type of packet exists */
                /* do not handle ACK (110), let server side to deal with it */
                if(strcmp(threadParam->data_buffer[endsysEthx].packet_q->next->packet->PacketType, "100") == 0){
                    //snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): forward File with type %s from %s to endsystem %s through ethx %d\n",
                    printf("sockclient(0x%x): forward File with type %s from %s to endsystem %s through ethx %d\n",
                        pthread_self(), threadParam->data_buffer[endsysEthx].packet_q->next->packet->PacketType,
                        threadParam->data_buffer[endsysEthx].packet_q->next->packet->RouterID, remote_server, endsysEthx);
                    //logging(LOGFILE, logmsg);

                    sendBufferData(clientfd, &threadParam->data_buffer[endsysEthx]);

                    needACK = 1;
                }
            }
            pthread_mutex_unlock(&threadParam->data_buffer[endsysEthx].lock_buffer);

            if(needACK == 1){
                    Packet *packet_ack;
                    packet_ack= (Packet *)malloc(sizeof(Packet));

                    /* got ack from endsys for each of the packet */
                    Recv(clientfd, packet_ack, sizeof(Packet), MSG_NOSIGNAL);

                    /* only add data ack (110) to buffer */
                    if(strcmp(packet_ack->PacketType, "110") == 0){
                        // data, addBuff();
                        //Thans_Data trans_data = (Trans_Data)packet->Data;
                        //getEthx(router, trans_data.des_ip); // calculate the out interface according to the routing table
                        //snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): got ack data from %s, enqueue...\n", 
                        printf("serverthread(0x%x): got ack data from %s, enqueue...\n", 
                                 pthread_self(), packet_ack->RouterID);
                        //logging(LOGFILE, logmsg);
                        repackData(router, packet_ack);
                        pthread_mutex_lock(&threadParam->data_buffer[endsysEthx].lock_buffer);
                        addBufferData(threadParam, packet_ack);
                        pthread_mutex_unlock(&threadParam->data_buffer[endsysEthx].lock_buffer);
                    }
                    else{
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): data is not ack.\n", pthread_self());
                        logging(LOGFILE, logmsg);
                    }
            }

            usleep(1); //sleep some time for lock relase
        }
    }
    /* other routers */
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
    
            while(1){
    
                gettimeofday(&timer, &tzp);
                time_t now = timer.tv_sec;
    
                /* read data from buffer */
                if(threadParam->data_buffer[ethx].buffsize > 0){
                    snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): threadParam->data_buffer[%d].buffsize: %d, first item type %s\n", 
                            pthread_self(), ethx, threadParam->data_buffer[ethx].buffsize, 
                            threadParam->data_buffer[ethx].packet_q->next->packet->PacketType);
                    logging(LOGFILE, logmsg);
    
                    /* Data or ACK packet */
                    if(strcmp(threadParam->data_buffer[ethx].packet_q->next->packet->PacketType, "100") == 0 ||
                            strcmp(threadParam->data_buffer[ethx].packet_q->next->packet->PacketType, "110") == 0){
                        //snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): forward Data %s packet to %s\n", 
                        printf("sockclient(0x%x): forward Data %s packet to %s\n", 
                                pthread_self(), threadParam->data_buffer[ethx].packet_q->next->packet->RouterID, remote_server);
                        //logging(LOGFILE, logmsg);
                        pthread_mutex_lock(&threadParam->data_buffer[ethx].lock_buffer);
                        sendBufferData(clientfd, &threadParam->data_buffer[ethx]);
                        pthread_mutex_unlock(&threadParam->data_buffer[ethx].lock_buffer);
                    }
    
                }

                if(threadParam->lsa_buffer[ethx].buffsize > 0){
                    snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): threadParam->lsa_buffer[%d].buffsize: %d, first item type %s\n",
                            pthread_self(), ethx, threadParam->lsa_buffer[ethx].buffsize,
                            threadParam->lsa_buffer[ethx].packet_q->next->packet->PacketType);
                    logging(LOGFILE, logmsg);

                    /* LSA packet */
                    if(strcmp(threadParam->lsa_buffer[ethx].packet_q->next->packet->PacketType, "010") == 0){
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): forward LSA %s packet to %s\n",
                                pthread_self(), threadParam->lsa_buffer[ethx].packet_q->next->packet->RouterID, remote_server);
                        logging(LOGFILE, logmsg);
                        pthread_mutex_lock(&threadParam->lsa_buffer[ethx].lock_buffer);
                        sendBufferLSA(clientfd, &threadParam->lsa_buffer[ethx]);
                        pthread_mutex_unlock(&threadParam->lsa_buffer[ethx].lock_buffer);
                    }
                }


                /* sending other packet */
                /* hello message */
                if(now % hello_interval ==0){
                    if(hello_sent == 0){
                        now1 = now;
                        hello_sent = 1;
        
                        pthread_mutex_lock(&threadParam->lock_send); // Critical section to read port files
                        sendHello(clientfd, router, atoi(portstr));
                        pthread_mutex_unlock(&threadParam->lock_send); // Critical section end
    
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
        
                        pthread_mutex_lock(&threadParam->lock_send); // Critical section to read port files
                        sendPing(clientfd, router, timer, ethx);
                        pthread_mutex_unlock(&threadParam->lock_send); // Critical section end
    
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
                        pthread_mutex_lock(&threadParam->lock_send); // Critical section to read port files
                        //printf("sockclient(0x%x): send LSA packet from %s to %s\n", pthread_self(), addrstr, remote_server);
                        sendNewLSA(clientfd, threadParam, ls_sequence_number, timer);
                        pthread_mutex_unlock(&threadParam->lock_send); // Critical section end
                        snprintf(logmsg, sizeof(logmsg), "sockclient(0x%x): LSA packet sent.\n", pthread_self());
                        logging(LOGFILE, logmsg);
                    }
                    if(now != now2){
                        lsa_sent = 0;
                    }
                }
    
                usleep(1); //sleep some time for lock relase
     
            } //endof while(1)
    
        } // endof if(strcmp(neighbor_reply
    } // endof if endsysEthx

    close(clientfd);
    pthread_exit(0);
}
