/*************************************************************************/
/* This program creates a well-known Internet domain socket.             */
/* Each time it accepts a connection, it forks a child to print          */
/* out a message from the client. It then goes on to accept new          */
/* connections. The child executes recho.                                */
/* To execute this program :                                             */
/*                                                                       */
/* At the local  machine:                                                */
/* 1. cc -o ex6a ex6a.c OR gcc -o ex6a ex6a.c -lsocket                   */ 
/* 2. cc -o recho ex6b.c OR gcc -o recho ex6b.c                          */
/*                                                                       */
/* At the remote machine:                                                */
/* 3. cc -o ex6c ex6c.c OR gcc -o ex6c ex6c.c -lsocket -lnsl             */ 
/* (-lsocket and -lnsl may not be required for gcc in all machines)      */
/*                                                                       */
/* local> ex6a (or ./ex6a)                                               */
/*         socket has port 1892                                          */
/*                      remote> ex6c (or ./ex6c) local.cs.pitt.edu 1892  */
/* (where local.cs.pitt.edu represents the hostname running ex6a, such   */ 
/*   as unixs4.cis.pitt.edu)                                             */
/*************************************************************************/
#include "socket.h"
#define BUFFER_SIZE 1024

int socket_printIP()
{
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  char hostname[1024];

  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  errcode = getaddrinfo (hostname, NULL, &hints, &res);
  if (errcode != 0)
    {
      perror ("getaddrinfo");
      return -1;
    }

  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
          break;
        }
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      printf ("STARTUP: IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
              addrstr, res->ai_canonname);
      res = res->ai_next;
    }
}

char* socket_getIP()
{
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  char hostname[1024];

  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  errcode = getaddrinfo (hostname, NULL, &hints, &res);
  if (errcode != 0)
    {
      perror ("getaddrinfo");
      return "\0";
    }

  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
          break;
        }
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      res = res->ai_next;
    }
return addrstr;
}

int
getaddr(char *hostname, char *addrstr)
{
  struct addrinfo hints, *res;
  int errcode;
  //char addrstr[100];
  void *ptr;

  //char hostname[1024];

  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  errcode = getaddrinfo (hostname, NULL, &hints, &res);
  if (errcode != 0)
    {
      perror ("getaddrinfo");
      return -1;
    }

  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
          break;
        }
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      res = res->ai_next;
    }

}

char* convertPacketToChar(struct packet pkg)
{
	char* buffer = malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
	memcpy(buffer,pkg.router_ID, 17);
	memcpy(buffer + 17, pkg.packet_type, 4);
	memcpy(buffer + 21, pkg.src_IP, 33);
	memcpy(buffer + 54, pkg.dest_IP, 33);
	memcpy(buffer + 87 ,pkg.length, 11);
	memcpy(buffer + 98, pkg.data, atoi(pkg.length) + 1);
	memcpy(buffer + 99 + atoi(pkg.length), pkg.packet_life, 5);
	memcpy(buffer + 104 + atoi(pkg.length), pkg.checksum, 33);
	return buffer;
}

struct packet convertCharToPacket(char* buffer)
{
	struct packet pkg;
	memcpy(pkg.router_ID, buffer, 17);	
	memcpy(pkg.packet_type, buffer + 17, 4);
	memcpy(pkg.src_IP, buffer + 21, 33);
	memcpy(pkg.dest_IP, buffer + 54 ,33);
	memcpy(pkg.length, buffer + 87, 11);
	pkg.data = malloc( atoi(pkg.length) + 1);
	memcpy(pkg.data, buffer + 98, atoi(pkg.length) + 1);
	memcpy(pkg.packet_life,buffer + 99 + atoi(pkg.length), 5);
	memcpy(pkg.checksum,buffer + 104 + atoi(pkg.length), 33);
	return pkg;
}

void socket_rcvFile()
{
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    int sockfd, client_fd, desc_ready;

    sin_size=sizeof(client_sockaddr);

    /*Data structure to handle timeout*/
    struct timeval before, timer, *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set, test_set;
    int maxfd, nready, nbytes;

    /* create socket */
    sockfd = Socket(AF_INET,SOCK_STREAM,0);

    /* set parameters for sockaddr_in */
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(0); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = INADDR_ANY; //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    Bind(sockfd, server_sockaddr);

    int port;
    char str[6], hostname[1024], addrstr[100];
    getaddr(hostname, addrstr); //get hostname and ip
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    sprintf(str, "%d", port); // int to str
    
    Listen(sockfd, 5);
    printf("STARTUP: Server is setup on port %d\n", port);
    if(fork() == 0)
	{
    while(1)
	{
    		client_fd = Accept(sockfd, client_sockaddr, sin_size);
		
    		char buff[BUFFER_SIZE];
    		/* Receive from the remote side */
    		memset(buff, 0, sizeof(buff));
    		Recv(client_fd,buff,sizeof(buff),0);
    		//Send(client_fd, buff, sizeof(buff),0);
    		close(client_fd);
		lsrp_incomingmessage(convertCharToPacket(buff));	
	}
	}
}

void socket_sendFile(char * hostname, int port, struct packet pkg)
{
    int sockfd,sendbytes,recvbytes;
    char buff[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    struct hostent *host;
    struct sockaddr_in sockaddr;

    /*address resolution*/
    if ((host = gethostbyname(hostname)) == NULL){
        perror("gethostbyname");
        exit(1);
    }

    /*create socket*/
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);         

    /*connect to server*/
    Connect(sockfd,sockaddr,sizeof(sockaddr));

    /*send message*/
    memset(buff, 0, sizeof(buff));
    memcpy(buff,convertPacketToChar(pkg),BUFFER_SIZE);
    sendbytes = Send(sockfd,buff,sizeof(buff), 0);
    
    //Recv(sockfd,buff,sizeof(buff), 0); 
    close(sockfd);
    free(buff);
    free(pkg.data);
    sleep(2);

}
