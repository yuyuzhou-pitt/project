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
#include "../packet/packet.h"

/*wrap socket*/
int Socket(int family, int type, int protocal){
    int sockfd;
    if((sockfd = socket(family, type, protocal)) < 0){
        perror("socket");
        return -1;
    }
    printf("Socket id = %d\n",sockfd);

    return sockfd;
}

/*wrap bind*/
int Bind(int sockfd, struct sockaddr_in sockaddr){
    int n;
    if((n = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr))) < 0){
        perror("bind");
        return -1;
    }
    printf("Bind success!\n");

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
    printf("socket has port %d\n", port); /* Display port number */

    return port;
}

/*wrap listen*/
int Listen(int sockfd, int max_que_comm_nm){
    int n;
    if((n = listen(sockfd, max_que_comm_nm) < 0)){
         perror("listen");
         return -1;
    }
    printf("Listening....\n");
    return n;
}

/*wrap accept*/
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int client_fd;
    if ((client_fd = accept(sockfd,(struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("accept");
        return -1;
    }
    printf("New incoming connection - %d\n", client_fd);
    return client_fd;
}

/*wrap recvfrom*/
int Recvfrom(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd, packet, size, flag, (struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("recvfrom");
        return -1;
    }    
    printf("Recvfrom message: %s\n", packet);
    return recvbytes;
}

/*wrap sendto*/
int Sendto(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, packet, size, flag, (struct sockaddr *)&sockaddr, sin_size)) < 0){
        perror("sendto");
        return -1;
    }
    printf("Sendto message: %s\n", packet);
    return sendbytes;
}

/*wrap recv*/
int Recv(int sockfd, Packet *packet, int size, int flag){
    int recvbytes;
    if ((recvbytes = recv(sockfd, packet, size, flag)) < 0){
        perror("recv");
        return -1;
    }
    printf("Recv message from: %s\n", packet->RouterID);
    return recvbytes;
}

/*wrap send*/
int Send(int sockfd, Packet *packet, int size, int flag){
    int sendbytes;
    if ((sendbytes = send(sockfd, packet, size, flag)) < 0){
        perror("send");
        return -1;
    }
    printf("Send message: %s\n",packet->RouterID);
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
        return -1;
    }
    return n;
}

/*write file*/
int writeFile(char *str, int size, char *file){
    FILE *fp;
    if ((fp = fopen(file,"w")) < 0){
        printf("writefile: Failed to open file: %s\n", file);
        return;
    }

    if((fwrite(str, 1, size, fp))<0){
        printf("writefile: Failed to write file %s.", file);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*read file*/
int readFile(char *str, int size, char *file){
    FILE *fp;

    if(access(file, F_OK) < 0) {
        printf("readfile: File not found: %s\n", file);
        return -1;
    }

    if ((fp = fopen(file,"r")) < 0){
        printf("readfile: Failed to open file: %s\n", file);
        return;
    }

    if((fgets(str, size, fp))<0){
        printf("readfile: Failed to read file: %s\n", file);
        return -1;
    }

    fclose(fp);

    return 0;
}

/*unlink port file*/
int unlinkPortFile(char *file){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, ".");
    strcat(hostfile, file);

    if(unlink(hostfile) < 0){
        printf("Failed to unlink file: %s \n", file);
        return -1;
    }

    return 0;
}

/*write port to host file*/
int writePort(int port, char *hostip){

    char portstr[6];
    sprintf(portstr, "%d", port); // int to str

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, ".");
    strcat(hostfile, hostip);

    if(writeFile(portstr, sizeof(portstr), hostfile) < 0){
        perror("writeport");
        return -1;
    }
    printf("write port %s to file: %s\n", portstr, hostfile);

    return 0;
}

/*get port from host file*/
int getPort(char *portstr, char *ipfile){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    int size = 6;
    strcpy(hostfile, ".");
    strcat(hostfile, ipfile);

    if(readFile(portstr, size, hostfile) < 0){
        perror("getport");
        return -1;
    }
    printf("port on %s is: %s\n", ipfile, portstr);

    return 0;
}
