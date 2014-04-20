#ifndef __LIBSOCKET_H__
#define __LIBSOCKET_H__

#include <netinet/in.h>
#include "../router/packet/packet.h"

int Socket(int family, int type, int protocal);
int Bind(int sockfd, struct sockaddr_in sockaddr);
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Listen(int sockfd, int max_que_comm_nm);
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Recvfrom(int sockfd, Packet *packet, int size, int flags, struct sockaddr_in sockaddr, int sin_size);
int Sendto(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size);
int Recv(int sockfd, Packet *packet, int size, int flags);
int Send(int sockfd, Packet *packet, int size, int flag);
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int writeFile(char *str, int size, char *file);
int readFile(char *str, int size, char *file);

#endif 
