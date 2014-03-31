#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include "libsocket.h"

/*wrap socket*/
int Socket(int family, int type, int protocal){
    int sockfd;
    if((sockfd = socket(family, type, protocal)) < 0){
        perror("socket");
        exit(1);
    }
    printf("Socket id = %d\n",sockfd);

    return sockfd;
}

/*wrap bind*/
int Bind(int sockfd, struct sockaddr_in sockaddr){
    int n;
    if((n = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr))) < 0){
        perror("bind");
        exit(1);
    }
    printf("Bind success!\n");

    return n;
}

/*got assigned sock port*/
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n, port;
    if((n = getsockname(sockfd, (struct sockaddr *)&sockaddr,&sin_size)) < 0){
        perror("getsockname");
        exit(1);
    }
    port = ntohs(sockaddr.sin_port);
    printf("socket has port %d\n",port); /* Display port number */

    return port;
}

/*wrap listen*/
int Listen(int sockfd, int max_que_comm_nm){
    int n;
    if((n = listen(sockfd, max_que_comm_nm) < 0)){
         perror("listen");
         exit(1);
    }
    printf("Listening....\n");
    return n;
}
   
/*wrap accept*/
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int client_fd;
    if ((client_fd = accept(sockfd,(struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("accept");
        exit(1);
    }
    printf("New incoming connection - %d\n", client_fd);
    return client_fd;
}

/*wrap recvfrom*/
int Recvfrom(int sockfd, char buff[], int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd,&buff, size, flag, (struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("recv");
        exit(1);
    }    
    printf("Received a message: msg=%s\n", &buff);
    return recvbytes;
}

/*wrap sendto*/
int Sendto(int sockfd, char buf[], int buf_size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, buf, buf_size, flag, (struct sockaddr *)&sockaddr, sin_size)) < 0){
        perror("send");
        exit(1);
    }
    printf("send message :%s\n",buf);
    return sendbytes;
}

/*wrap ioctl*/
int Ioctl(int sockfd, int request, int i){
    /* Set socket to be non-blocking. */
    int n;
    if((n = ioctl(sockfd, FIONBIO, (char *)&i)) < 0){
        perror("ioctl");
        exit(-1);
    }
    return n;
}

/*wrap connect*/
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n;
    if((n = connect(sockfd,(struct sockaddr *)&sockaddr,sin_size)) < 0){
        perror("connect");
        exit(1);
    }
    return n;
}
