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
#include "../router/packet/packet.h"
#include "../router/config/liblog.h"

/*wrap socket*/
int Socket(int family, int type, int protocal){
    int sockfd;
    if((sockfd = socket(family, type, protocal)) < 0){
        perror("socket");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Socket id = %d\n",sockfd);
    logging(LOGFILE, logmsg);

    return sockfd;
}

/*wrap bind*/
int Bind(int sockfd, struct sockaddr_in sockaddr){
    int n;
    if((n = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr))) < 0){
        perror("bind");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Bind success!\n");
    logging(LOGFILE, logmsg);

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
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "socket has port %d\n", port); /* Display port number */
    logging(LOGFILE, logmsg);

    return port;
}

/*wrap listen*/
int Listen(int sockfd, int max_que_comm_nm){
    int n;
    if((n = listen(sockfd, max_que_comm_nm) < 0)){
         perror("listen");
         return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Listening....\n");
    logging(LOGFILE, logmsg);
    return n;
}

/*wrap accept*/
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int client_fd;
    if ((client_fd = accept(sockfd,(struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("accept");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "New incoming connection - %d\n", client_fd);
    logging(LOGFILE, logmsg);
    return client_fd;
}

/*wrap recvfrom*/
int Recvfrom(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd, packet, size, flag, (struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("recvfrom");
        return -1;
    }    
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Recvfrom message: %s\n", packet->RouterID);
    logging(LOGFILE, logmsg);
    return recvbytes;
}

/*wrap sendto*/
int Sendto(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, packet, size, flag, (struct sockaddr *)&sockaddr, sin_size)) < 0){
        perror("sendto");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Sendto message: %s\n", packet->RouterID);
    logging(LOGFILE, logmsg);
    return sendbytes;
}

/*wrap recv*/
int Recv(int sockfd, Packet *packet, int size, int flag){
    int recvbytes;
    if ((recvbytes = recv(sockfd, packet, size, flag)) < 0){
        perror("recv");
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Recv message from: %s\n", packet->RouterID);
    //logging(LOGFILE, logmsg);
    return recvbytes;
}

/*wrap send*/
int Send(int sockfd, Packet *packet, int size, int flag){
    int sendbytes;
    if ((sendbytes = send(sockfd, packet, size, flag)) < 0){
        perror("send");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Send message: %s\n",packet->RouterID);
    logging(LOGFILE, logmsg);
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

/*write file*/
int writeFile(char *str, int size, char *file){
    FILE *fp;
    if ((fp = fopen(file,"w")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if((fwrite(str, 1, size, fp))<0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to write file %s.", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*read file*/
int readFile(char *str, int size, char *file){
    FILE *fp;
/*
    if(access(file, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }*/

    if ((fp = fopen(file,"r")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if((fgets(str, size, fp))<0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to read file: %s\n", file);
        logging(LOGFILE, logmsg);
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
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Failed to unlink file: %s \n", file);
        logging(LOGFILE, logmsg);
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

    //strcpy(hostfile, ".");
    strcat(hostfile, hostip);

    if(writeFile(portstr, sizeof(portstr), hostfile) < 0){
        perror("writeport");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "write port %s to file: %s\n", portstr, hostfile);
    logging(LOGFILE, logmsg);

    return 0;
}

/*get port from host file*/
int getPort(char *portstr, int size, char *ipfile){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    //strcpy(hostfile, ".");
    strcat(hostfile, ipfile);

    if(readFile(portstr, size, hostfile) < 0){
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "port on %s is: %s\n", ipfile, portstr);
    //logging(LOGFILE, logmsg);

    return 0;
}

/* mark port */
int markPort(char *filename, char *newline){

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, ".");
    strcat(hostfile, filename);

    if(writeFile(newline, strlen(newline), hostfile) < 0){
        perror("markport");
        return -1;
    }
}
