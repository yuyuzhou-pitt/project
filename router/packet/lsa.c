#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../../endsys/checksum.h"
#include "../config/config.h"
#include "../config/libfile.h"
#include "../config/liblog.h"

#define LSA 1
#include "packet.h"
#include "libqueue.h"
#define GRAPHOSPF ".graph.ospf"
#include "dijkstra.h"

Packet *genLSAMsg(ThreadParam *threadParam, int ls_sequence_number, struct timeval timer){
//Packet *genLSAMsg(Router *router, int ls_sequence_number, struct timeval timer, int port, char *remote_server){
    /*generate lsa message*/
    Router *router;
    router = threadParam->router;

    LSA_Msg lsa_msg;

    //printf("genLSAMsg: set Advertising_Router_ID: %s\n", router->router_id);
    snprintf(lsa_msg.Advertising_Router_ID, sizeof(lsa_msg.Advertising_Router_ID), "%s", router->router_id); // original src_ip
    lsa_msg.Advertising_Port_ID = threadParam->port;
    //snprintf(lsa_msg.Destination_Router_ID, sizeof(lsa_msg.Destination_Router_ID), "%s", remote_server); // original des_ip
    lsa_msg.LS_Age = timer.tv_sec;
    lsa_msg.LS_Sequence_Number = ls_sequence_number; // new ls_sequence_number is 0, add 1 when repack
    //lsa_msg.Length = 1//how ??
    lsa_msg.Number_of_Links = router->num_of_interface; 

    int i;
    for(i=0;i < router->num_of_interface;i++){
        //printf("genLSAMsg: set port id: %d\n", port);
        snprintf(lsa_msg.ls_link[i].Link_ID, sizeof(lsa_msg.ls_link[i].Link_ID), "%s", router->ethx[i].eth_id);
        snprintf(lsa_msg.ls_link[i].netmask, sizeof(lsa_msg.ls_link[i].netmask), "%s", router->ethx[i].netmask);
        snprintf(lsa_msg.ls_link[i].Direct_Link_Addr, sizeof(lsa_msg.ls_link[i].Direct_Link_Addr), "%s", router->ethx[i].direct_link_addr);
        /* check endsystem or not */
        if(router->ethx[i].direct_link_type == 1){
            snprintf(lsa_msg.ls_link[i].Direct_Link_EthID, sizeof(lsa_msg.ls_link[i].Direct_Link_EthID), "end-system-ethx ");
        }
        else{
            snprintf(lsa_msg.ls_link[i].Direct_Link_EthID, sizeof(lsa_msg.ls_link[i].Direct_Link_EthID), "%s", router->ethx[i].direct_link_eth_id);
        }
        lsa_msg.ls_link[i].PortID = router->ethx[i].direct_link_port;
        lsa_msg.ls_link[i].Availability = router->ethx[i].link_availability; //default as 1, set it as 0 if timeout or disabled
        lsa_msg.ls_link[i].Link_Cost = router->ethx[i].link_cost; //router->ethx[i].link_cost;
    }
    /*wrap into packet*/
    Packet *lsa_packet;
    lsa_packet = (Packet *)calloc(1, sizeof(Packet)); //Packet with LSA_Msg type Data

    snprintf(lsa_packet->RouterID, sizeof(lsa_packet->RouterID), "%s", router->router_id);
    snprintf(lsa_packet->PacketType, sizeof(lsa_packet->PacketType), "%s", "010"); // LSA Packets (010)

    lsa_packet->Data = (LSA_Msg) lsa_msg; // Data

    /*checksum*/
    snprintf(lsa_packet->PacketChecksum, sizeof(lsa_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) lsa_packet, sizeof(*lsa_packet)));

    return lsa_packet;
}


int installLSA(ThreadParam *threadParam, Packet *packet_req){
    Packet *packet;
    packet = (Packet *)packet_req;

    /* check whether the same LSA with new sequence number */
    int existedLSA = 0;

    int dbIndex;
    //printf("installLSA: got LSA packet from: %s\n", packet->Data.Advertising_Router_ID);
    for(dbIndex=0;dbIndex < threadParam->ls_db_size; dbIndex++){
        /* whether Advertising_Router_ID is the des_router_id */
        if(strcmp(threadParam->ls_db[dbIndex].des_router_id, packet->Data.Advertising_Router_ID) == 0){
            existedLSA = 1;
            /* do update if got bigger seq number */
            //printf("installLSA: this packet from %s in db already\n", packet->Data.Advertising_Router_ID);
            if(threadParam->ls_db[dbIndex].LS_Sequence_Number < packet->Data.LS_Sequence_Number){
                //printf("installLSA: do update as got bigger seq number %d from: %s\n", packet->Data.LS_Sequence_Number, packet->Data.Advertising_Router_ID);
                int i;
                for(i=0;i < packet->Data.Number_of_Links;i++){
                    /* update if interface exists */
                    if(strcmp(threadParam->ls_db[dbIndex].src_router_id, packet->Data.ls_link[i].Direct_Link_Addr) == 0){
                        snprintf(threadParam->ls_db[dbIndex].Link_ID, sizeof(threadParam->ls_db[dbIndex].Link_ID), "%s", packet->Data.ls_link[i].Direct_Link_EthID);
                        threadParam->ls_db[dbIndex].des_port_id = packet->Data.Advertising_Port_ID;
                        threadParam->ls_db[dbIndex].src_port_id = packet->Data.ls_link[i].PortID;
                        threadParam->ls_db[dbIndex].Availability = packet->Data.ls_link[i].Availability;
                        threadParam->ls_db[dbIndex].LS_Sequence_Number = packet->Data.LS_Sequence_Number;
                        threadParam->ls_db[dbIndex].Link_Cost = packet->Data.ls_link[i].Link_Cost;
                        threadParam->ls_db[dbIndex].LS_Age = packet->Data.LS_Age;
                    }
                }

            }
        }
    }

    if(existedLSA == 0){
        //printf("installLSA: install new LSA packet from %s\n", packet->Data.Advertising_Router_ID);
        /* add all new LS into db */
        int i;
        for(i=0;i < packet->Data.Number_of_Links;i++){
            /* skip if port is 0 (let genGraph do that) */ 
            //if(packet->Data.Advertising_Port_ID != 0 && packet->Data.ls_link[i].PortID != 0){
                //printf("installLSA: Advertising_Router_ID - PortID: %s - %s\n", packet->Data.Advertising_Router_ID, packet->Data.ls_link[i].PortID);
                dbIndex = threadParam->ls_db_size;
                snprintf(threadParam->ls_db[dbIndex].Link_ID, sizeof(threadParam->ls_db[dbIndex].Link_ID), "%s", packet->Data.ls_link[i].Direct_Link_EthID);
                snprintf(threadParam->ls_db[dbIndex].netmask, sizeof(threadParam->ls_db[dbIndex].netmask), "%s", packet->Data.ls_link[i].netmask);
                snprintf(threadParam->ls_db[dbIndex].src_router_id, sizeof(threadParam->ls_db[dbIndex].src_router_id), "%s", packet->Data.ls_link[i].Direct_Link_Addr);
                threadParam->ls_db[dbIndex].des_port_id = packet->Data.Advertising_Port_ID;
                snprintf(threadParam->ls_db[dbIndex].des_router_id, sizeof(threadParam->ls_db[dbIndex].des_router_id), "%s", packet->Data.Advertising_Router_ID);
                threadParam->ls_db[dbIndex].src_port_id = packet->Data.ls_link[i].PortID;
                threadParam->ls_db[dbIndex].Availability = packet->Data.ls_link[i].Availability;
                threadParam->ls_db[dbIndex].LS_Sequence_Number = packet->Data.LS_Sequence_Number;
                threadParam->ls_db[dbIndex].Link_Cost = packet->Data.ls_link[i].Link_Cost;
                threadParam->ls_db[dbIndex].LS_Age = packet->Data.LS_Age;
        
                threadParam->ls_db_size++;
            /*}
            else{
                char logmsg[128];
                snprintf(logmsg, sizeof(logmsg), "installLSA: either src_port (%d) or des_port (%d) is empty, skip.\n", \
                         packet->Data.ls_link[i].PortID, packet->Data.Advertising_Port_ID);
                logging(LOGFILE, logmsg);
            }*/
        }
    }

    return 0;
}

int sendNewLSA(int sockfd, ThreadParam *threadParam, int ls_sequence_number, struct timeval timer){
    Packet *lsa_packet;

    lsa_packet = genLSAMsg(threadParam, ls_sequence_number, timer); // msg to be sent out
    /* add into LS DB before send out */
    //pthread_mutex_lock(&threadParam->lock_ls_db);
    installLSA(threadParam, lsa_packet);
    //pthread_mutex_unlock(&threadParam->lock_ls_db);

    //printf("sendNewLSA: send LSA packet %s with type %s\n", lsa_packet->RouterID, lsa_packet->PacketType);
    //printf("sendLSA: lsa_packet->PacketType = %s\n", lsa_packet->PacketType);
    return Send(sockfd, lsa_packet, sizeof(Packet), MSG_NOSIGNAL);
}

/* replace the router_id */
int repackLSA(Router *router, Packet *packet){
    snprintf(packet->RouterID, sizeof(packet->RouterID), "%s", router->router_id);
}

int sendBufferLSA(int sockfd, Packet_Buff *buffer){
    int r;
    Packet *lsa_packet;
    char logmsg[128]; 
    if(buffer->buffsize > 0){
        lsa_packet = (Packet *)dequeue(buffer->packet_q);
        r = Send(sockfd, lsa_packet, sizeof(Packet), MSG_NOSIGNAL);
        snprintf(logmsg, sizeof(logmsg), "sendBufferLSA: packet sent: %s\n", lsa_packet->Data.Advertising_Router_ID);
        logging(LOGFILE, logmsg);
        buffer->buffsize--;
        //printf("sendBufferLSA:buffer->buffsize = %d\n",buffer->buffsize);
    }
    else{
        snprintf(logmsg, sizeof(logmsg), "sendBufferLSA: Buffer is empty.");
        logging(LOGFILE, logmsg);
        return -1;
    }
    return r;
}

/* add LSA to flood buffer, except the ethx from which it received the LSA. */
int addBufferFlood(ThreadParam *threadParam, Packet *packet, int ethx, struct timeval timer){
    int i;
    int isDirect = 0;
    char logmsg[128]; 
    /* TODO: prevent loop */
    //printf("addBufferflood:threadParam->router->num_of_interface = %d\n",threadParam->router->num_of_interface);
    /* do nothing if the time past certain rounds */
    if (timer.tv_sec - packet->Data.LS_Age > threadParam->router->ls_age_limit){
        snprintf(logmsg, sizeof(logmsg), "addBufferFlood: LS_Age_limit reached, drop packet original from %s.", packet->Data.Advertising_Router_ID);
        logging(LOGFILE, logmsg);
        return -1;
    }

    for (i=0;i < threadParam->router->num_of_interface; i++){
        /* prevent loop before enqueue: 
         * 1) do nothing if original Advertising_Router_ID is the target */
        /* 2) do nothing if original Advertising_Router_ID is me */
        /* 3) do nothing if the interface connectes to end system (direct_link_type is 1) */
        /* 4) do not send back to the RouterID comes from */
        //if(strcmp(threadParam->router->ethx[i].direct_link_addr, packet->Data.Advertising_Router_ID) != 0){
        if(strcmp(threadParam->router->ethx[i].direct_link_addr, packet->Data.Advertising_Router_ID) != 0 && \
           strcmp(threadParam->router->router_id, packet->Data.Advertising_Router_ID) != 0 && \
           strcmp(threadParam->router->ethx[i].direct_link_addr, packet->RouterID) != 0 && \
           threadParam->router->ethx[i].direct_link_type != 1) {

            /* do not use same link to send back */
            if(i != ethx){
                pthread_mutex_lock(&threadParam->lsa_buffer[i].lock_buffer);
                enqueue(threadParam->lsa_buffer[i].packet_q, packet);
                //printf("addBufferflood:threadParam->lsa_buffer[i].packet_q->next->packet->RouterID = %s\n",threadParam->lsa_buffer[i].packet_q->next->packet->RouterID);
                threadParam->lsa_buffer[i].buffsize++;
                pthread_mutex_unlock(&threadParam->lsa_buffer[i].lock_buffer);
                snprintf(logmsg, sizeof(logmsg), "addBufferflood:threadParam->lsa_buffer[%d].buffsize = %d\n",i, threadParam->lsa_buffer[i].buffsize);
                logging(LOGFILE, logmsg);
            }

        }
    }
    return 0;
}

/* for dijkstra algo */
int genGraph(ThreadParam *threadParam, char *ipstr){
    char graphfile[32];
    memset(graphfile, 0, sizeof(graphfile));
    snprintf(graphfile, sizeof(graphfile), ".%s", ipstr);
    strncat(graphfile, GRAPHOSPF, strlen(GRAPHOSPF));
    char graphStr[2048];
    memset(graphStr, 0, sizeof(graphStr));

    FILE *fp;
    if((fp=fopen(graphfile, "w"))<0){
        char logmsg[128]; 
        snprintf(logmsg, sizeof(logmsg), "genGraph: fail to open file: %s\n", GRAPHOSPF);
        logging(LOGFILE, logmsg);
        return -1;
    }

    Graph_Line *line;
    line = threadParam->graph_line;

    memset(line, 0, 128*sizeof(Graph_Line));
    int i,j,iLine,iSize = 0;

    /* generate graph table */
    for(i=0;i < threadParam->ls_db_size;i++){
        /* skip if 
         * 1) port is 0 
         * 2) connects to end system
         * 3) interface is disabled (Avail is 0) */
        if(threadParam->ls_db[i].src_port_id != 0 && threadParam->ls_db[i].des_port_id != 0 &&
           strcmp(threadParam->ls_db[i].Link_ID, "end-system-ethx ") != 0 &&
           threadParam->ls_db[i].Availability != 0){
            int isExist = 0;
            char templine[16];
            memset(templine, 0, sizeof(templine));
            iLine = iSize; // add new line by default
    
            /* node exists or not */
            for(j=0;j<iSize;j++){
                if(line[j].lineId == threadParam->ls_db[i].src_port_id){
                    isExist = 1;
                    iLine = j;
                    /* update line */
                }
            }
    
            if(isExist == 0){
                /*add new line*/
                line[iLine].lineId = threadParam->ls_db[i].src_port_id;
                snprintf(line[iLine].lineStr, sizeof(line[iLine].lineStr), "%d", line[iLine].lineId);
                iSize++;
            }
    
            /* add <des_node>-<cost> */
            snprintf(templine, sizeof(templine), ":%d-%d", threadParam->ls_db[i].des_port_id, \
               threadParam->ls_db[i].Link_Cost.tv_sec * 1000000 + threadParam->ls_db[i].Link_Cost.tv_usec);
            strncat(line[iLine].lineStr, templine, strlen(templine));
            //printf("genGraph:line[%d].lineStr: %s\n", iLine, line[iLine].lineStr);
        }
        else{
            char logmsg[128];
            snprintf(logmsg, sizeof(logmsg), "genGraph: either src_port (%d) or des_port (%d) is empty, or it's a end-system-ethx. Skip.\n", \
                     threadParam->ls_db[i].src_port_id != 0, threadParam->ls_db[i].des_port_id);
            logging(LOGFILE, logmsg);
        }

    }// end for(i=0;

    threadParam->graph_line_size = iSize;
    for(j=0;j<iSize;j++){
        char tempStr[128];
        memset(tempStr, 0, sizeof(tempStr));
        if(j == iSize - 1){
            /* no new line at the end of file */
            snprintf(tempStr, sizeof(tempStr), "%s;",line[iSize-1].lineStr);
        }
        else{
            snprintf(tempStr, sizeof(tempStr), "%s;\n",line[j].lineStr);
        }
        strncat(graphStr, tempStr, strlen(tempStr));
    }

    //printf("genGraph:\n%s\n\n", graphStr);

    fwrite(strstrip(graphStr), 1 , strlen(strstrip(graphStr)) , fp );

    fclose(fp);

    return 0;
}

int portToHost(char *routerid, ThreadParam *threadParam, int port){
    int i;
    for(i=0;i < threadParam->ls_db_size;i++){
        if(port == threadParam->ls_db[i].des_port_id){
            strncpy(routerid, threadParam->ls_db[i].des_router_id, strlen(threadParam->ls_db[i].des_router_id));
            //routerid = threadParam->ls_db[i].des_router_id;
            return 0;
        }
    }

    return -1; // no port found
}

int portToInterface(char *interface, ThreadParam *threadParam, int src_port, int des_port){
    int i;
    for(i=0;i < threadParam->ls_db_size;i++){
        if(des_port == threadParam->ls_db[i].des_port_id && src_port == threadParam->ls_db[i].src_port_id){
            strncpy(interface, threadParam->ls_db[i].Link_ID, strlen(threadParam->ls_db[i].Link_ID));
            //interface = threadParam->ls_db[i].Link_ID;
            return 0;
        }
    }

    return -1; // no port found
}

int portToNetmask(char *netmask, ThreadParam *threadParam, int src_port, int des_port){
    int i;
    for(i=0;i < threadParam->ls_db_size;i++){
        if(des_port == threadParam->ls_db[i].des_port_id && src_port == threadParam->ls_db[i].src_port_id){
            strncpy(netmask, threadParam->ls_db[i].netmask, strlen(threadParam->ls_db[i].netmask));
            //netmask = threadParam->ls_db[i].netmask;
            return 0;
        }
    }

    return -1; // no port found
}

/* generate routing table based on ls database and dijkstra algo */
int genRouting(ThreadParam *threadParam, char *ipstr){
    char graphfile[32];
    memset(graphfile, 0, sizeof(graphfile));
    snprintf(graphfile, sizeof(graphfile), ".%s", ipstr);
    strncat(graphfile, GRAPHOSPF, strlen(GRAPHOSPF));

    int gateway, metric;
    
    records nlist[MAX_NODES+1]={0}; //create array of records and mark the end of the array with zero.
    nlist[MAX_NODES+1].nid=0;

    int count; //stores number of nodes

    count = scanfile(graphfile, nlist);
    djkstra(nlist, count);
    //view(nlist);

    char tmpStr[32];
    memset(tmpStr, 0, sizeof(tmpStr));

    int rRouterid, rNetmask, rGatewayid, rInterface;
    char routerid[32], netmask[32], gatewayid[2], interface[32];
    memset(routerid, 0, sizeof(routerid));
    memset(netmask, 0, sizeof(netmask));
    memset(gatewayid, 0, sizeof(gatewayid));
    memset(interface, 0, sizeof(interface));

    memset(threadParam->routing, 0, 128*sizeof(Routing_Table));
    int k = 0;
    //int k = threadParam->routing_size;
    int i, r;
    for(i=0;i < threadParam->graph_line_size;i++){
        /* except self */
        if(threadParam->port != threadParam->graph_line[i].lineId){
            /* skip if no route */
            if(r = min_route(threadParam->port, threadParam->graph_line[i].lineId, &gateway, &metric, nlist) >= 0){
                //printf("src=%d, dest=%d, gateway=%d, metric=%d\n", threadParam->port, threadParam->graph_line[i].lineId, gateway, metric);

                rRouterid = portToHost(routerid, threadParam, threadParam->graph_line[i].lineId);
                rGatewayid = portToHost(gatewayid, threadParam, gateway);
                rInterface = portToInterface(interface, threadParam, threadParam->port, gateway);
                rNetmask = portToNetmask(netmask, threadParam, threadParam->port, threadParam->graph_line[i].lineId);

                /* skip if no ls_db exists */
                //if(rRouterid != -1 && rInterface != -1 && rGatewayid != -1 && rNetmask != -1){
                    strncpy(threadParam->routing[k].Destination, routerid, strlen(routerid));
                    strncpy(threadParam->routing[k].GenMask, netmask, strlen(netmask));
                    strncpy(threadParam->routing[k].Gateway, gatewayid, strlen(gatewayid));
                    threadParam->routing[k].Metric = metric;
                    strncpy(threadParam->routing[k].Interface, interface, strlen(interface));
                    k++;
                //} //end of if(portToportToInterface
            } //end of if if(r = min_route
        } // end of  if(threadParam->port
    }// end for(i=0;
    threadParam->routing_size = k;
    //printf("genRouting: routing_size:%d\n", threadParam->routing_size);
}

/* for terminal */
int showRouting(ThreadParam *threadParam){
    int i;
    fprintf(stdout, "\nshowRouting: routing_size: %d\n", threadParam->routing_size);
    fprintf(stdout, "Destination\tGateway    \tMetric(us)\tIface\n");
    for(i=0;i< threadParam->routing_size;i++){
        fprintf(stdout, "%s\t%s\t%d\t%s\n", threadParam->routing[i].Destination, \
                threadParam->routing[i].Gateway, \
                threadParam->routing[i].Metric, threadParam->routing[i].Interface);
    }
}

/* for terminal */
int showLSDB(ThreadParam *threadParam){
    int i;
    fprintf(stdout, "\nshowLSDB: ls_db_size: %d\n", threadParam->ls_db_size);
    fprintf(stdout, "Link_ID            \tsrc_router\tsrc_port\tdes_router\tdes_port\tAvail\tSeq\tCost(s:us)\tLS_Age    \n");
    /*  10.0.0.1	10.0.0.4	10.0.0.5	36747	1	3	9999:0	1397584710 */
    for(i=0;i < threadParam->ls_db_size;i++){
        fprintf(stdout, "%s\t%s\t%d    \t%s\t%d    \t%d\t%d\t%d:%d\t%d\n", \
               threadParam->ls_db[i].Link_ID, threadParam->ls_db[i].src_router_id, \
               threadParam->ls_db[i].src_port_id, threadParam->ls_db[i].des_router_id, \
               threadParam->ls_db[i].des_port_id, threadParam->ls_db[i].Availability, \
               threadParam->ls_db[i].LS_Sequence_Number, threadParam->ls_db[i].Link_Cost.tv_sec, \
               threadParam->ls_db[i].Link_Cost.tv_usec, threadParam->ls_db[i].LS_Age);
    }
    return 0;
}
