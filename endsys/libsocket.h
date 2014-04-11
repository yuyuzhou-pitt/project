#ifndef __LIBSOCKET_H__
#define __LIBSOCKET_H__

typedef struct bat{
    char group[20];
    int  num;
}my;

int Socket(int family, int type, int protocal);
int Bind(int sockfd, struct sockaddr_in sockaddr);
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Listen(int sockfd, int max_que_comm_nm);
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Recvfrom(int sockfd, char buff[], int size, int flags, struct sockaddr_in sockaddr, int sin_size);
int Sendto(int sockfd, char buf[], int buf_size, int flag, struct sockaddr_in sockaddr, int sin_size);
int Recv(int sockfd, char buff[], int size, int flags);
int Send(int sockfd, char buf[], int buf_size, int flag);
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size);

#endif
