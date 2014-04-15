#ifndef __PACKET_H__
#define __PACKET_H__

#define ETHX 128

typedef struct LS_Link_Status{
    char Link_ID[32]; // use eth_id in cfg, Identifies the ID of the Link
    char Direct_Link_Addr[32];
    int PortID;
    int Availability; // the status of availability
    struct timeval Link_Cost;// the status of the cost in micro seconds
}LS_Link;
 
typedef struct Neighbors_Acquisiton_Message{
    char NeighborAcqType[4]; // 0~4
    int PortID;
    int HelloInterval; // for alive, default 40s
    int UpdateInterval; // for LSA update, default
    char ProtocolVersion[4]; // routers with different version should not be neighbors
    LS_Link ls_link[ETHX];
    char placeholder[63]; // the last two parameters are to hold place for Packet transfer
}Neighbor_Msg;

typedef struct Hello_Message{
    int PortID; // hello message, value is port ID
}Hello_Msg;

typedef struct Link_State_Advertisement_Message{
    char Advertising_Router_ID[32]; // the originating router of the LSA (src_ip)
    time_t LS_Age; // the number of seconds since the LSA was originated, reset every time a new instance of the same LSA is received.
    int LS_Sequence_Number; // to distinguish between instances of the same LSA.
    int Length; //the length, in bytes, of the LSA counting both LSA header and contents.
    int Number_of_Links; // Identifies the number of links reported in the LSA.
    LS_Link ls_link[ETHX]; //ETHX defined in config.h
}LSA_Msg;

typedef struct Ping_Message{
    int ping_pong_bit; // 0 that means ping which needs a response, else 1 means pong and no response is needed.
    struct timeval timer; // the time stamp when send out 
    char src_ip[32]; //??
    char des_ip[32]; //??
    int packet_life; // in seconds
}Ping_Msg;

typedef struct Transfer_Data{
    char src_ip[32];
    char des_ip[32];
    int length;
    char data[1024];
    int packet_life; // in seconds
}Trans_Data;

/*all message will be wrapped into packet*/
typedef struct Packet{
    char RouterID[32]; // 0~31,default: LOOKBACK
    char PacketType[4]; // 32~34
#ifdef NEIGHBOR
    Neighbor_Msg Data; // can NOT be pointer as memory addresses are different on client and server
#elif HELLO
    Hello_Msg Data; //34 + len(data)
#elif LSA
    LSA_Msg Data; //34 + len(data)
#elif PING
    Ping_Msg Data; //34 + len(data)
#elif TRANSFILE
    Trans_Data Data; //34 + len(data)
#else
    //Hello_Msg Data; //34 + len(data)
    Neighbor_Msg Data; // default as Neighbor_Msg, which is the first message to exchange
#endif
    char PacketChecksum[32]; // crc32
}Packet;

//neighbor_acq_msg *genNeighborReq(char *filename, char *hostip, char *port);
//int genNeighborReply(char *filename, neighbor_acq_msg *neighbor_req, neighbor_acq_msg *neighbor_reply);

typedef struct Link_State_Database{
    char Link_ID[32]; // use eth_id in cfg, Identifies the ID of the Link
    char des_router_id[32];
    int des_port_id;
    char src_router_id[32];
    int src_port_id;
    int Availability; // the status of availability
    int LS_Sequence_Number;
    struct timeval Link_Cost;// the status of the cost in micro seconds
    time_t LS_Age;
}LS_DB;

#endif
