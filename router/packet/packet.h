#ifndef __PACKET_H__
#define __PACKET_H__

typedef struct Neighbors_Acquisiton_Message{
    char NeighborAcqType[4]; // 0~4
    char PortID[32];
    int HelloInterval; // for alive, default 40s
    int UpdateInterval; // for LSA update, default
    char ProtocolVersion[4]; // routers with different version should not be neighbors
}Neighbor_Msg;

typedef struct Hello_Message{
    int Hello; // hello message, value is always '1'
}Hello_Msg;

typedef struct  Link_State_Advertisement_Message{
    char NeighborAcqType[4]; // 0~4
    char PortID[32];
    char HelloInterval[4]; // for alive, default 40s
    char UpdateInterval[4]; // for LSA update, default
    char ProtocolVersion[4]; // routers with different version should not be neighbors
}LSA_Msg;

typedef struct Ping_Message{
    char NeighborAcqType[4]; // 0~4
    char PortID[32];
    char HelloInterval[4]; // for alive, default 40s
    char UpdateInterval[4]; // for LSA update, default
    char ProtocolVersion[4]; // routers with different version should not be neighbors
}Ping_Msg;

typedef struct Transfer_File{
    char filename[64];
    char filetype[8];
    char content[4096];
}Trans_File;

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
    Trans_File Data; //34 + len(data)
#else
    Neighbor_Msg Data; // default as Neighbor_Msg, which is the first message to exchange
#endif
    char PacketChecksum[32]; // crc32
}Packet;

//neighbor_acq_msg *genNeighborReq(char *filename, char *hostip, char *port);
//int genNeighborReply(char *filename, neighbor_acq_msg *neighbor_req, neighbor_acq_msg *neighbor_reply);

#endif
