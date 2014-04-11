/*
 * Wrap of socket fuctions.
*/

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
        return -1;
    }
    //printf("Socket id = %d\n",sockfd);

    return sockfd;
}

/*wrap bind*/
int Bind(int sockfd, struct sockaddr_in sockaddr){
    int n;
    if((n = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr))) < 0){
        perror("bind");
        return -1;
    }
    //printf("Bind success!\n");

    return n;
}

/*got assigned sock port*/
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n, port;
    if((n = getsockname(sockfd, (struct sockaddr *)&sockaddr,&sin_size)) < 0){
        perror("getsockname");
        return -1;
    }
    
    port = ntohs(sockaddr.sin_port);
    return port;
}

/*wrap listen*/
int Listen(int sockfd, int max_que_comm_nm){
    int n;
    if((n = listen(sockfd, max_que_comm_nm) < 0)){
         perror("listen");
         return -1;
    }
    //printf("Listening....\n");
    return n;
}

/*wrap accept*/
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int client_fd;
    if ((client_fd = accept(sockfd,(struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("accept");
        return -1;
    }
    //printf("New incoming connection - %d\n", client_fd);
    return client_fd;
}

/*wrap recvfrom*/
int Recvfrom(int sockfd, char buff[], int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd,buff, size, flag, (struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("recvfrom");
        return -1;
    }    
    printf("Recvfrom message: %s\n", buff);
    return recvbytes;
}

/*wrap sendto*/
int Sendto(int sockfd, char buf[], int buf_size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, buf, buf_size, flag, (struct sockaddr *)&sockaddr, sin_size)) < 0){
        perror("sendto");
        return -1;
    }
    printf("Sendto message: %s\n",buf);
    return sendbytes;
}

/*wrap recv*/
int Recv(int sockfd, char buff[], int size, int flag){
    int recvbytes;
    if ((recvbytes = recv(sockfd,buff, size, flag)) < 0){
        perror("recv");
        return -1;
    }
    printf("Recv message: %s\n", buff);
    return recvbytes;
}

/*wrap send*/
int Send(int sockfd, char buf[], int buf_size, int flag){
    int sendbytes;
    if ((sendbytes = send(sockfd, buf, buf_size, flag)) < 0){
        perror("send");
        return -1;
    }
    printf("Send message: %s\n",buf);
    return sendbytes;
}

/*wrap connect*/
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n;
    if((n = connect(sockfd,(struct sockaddr *)&sockaddr,sin_size)) < 0){
        perror("connect");
        return -1;
    }
    return n;
}

/*unlink port file*/
int unlinkPortFile(char *file){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, ".");
    strcat(hostfile, file);

    if(unlink(hostfile) < 0){
        perror("unlinkPortFile");
        return -1;
    }
    
    return 0;
}
