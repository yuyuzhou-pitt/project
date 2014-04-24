/*************************************************************************/
/*	This requres -lsocket -lnsl when linking the object              */ 
/*************************************************************************/
#define TRANSFILE 1
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

int socket_getIP(char *addrstr)
{
  struct addrinfo hints, *res;
  int errcode;
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
      res = res->ai_next;
    }
return 0;
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

Packet convertPacketToChar(struct packet pkg)
{

	Packet outgoing;
	//Old Code
	/*
	char* buffer = malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
	memcpy(buffer,pkg.router_ID, 17 + 16);
	memcpy(buffer + 17+ 16, pkg.packet_type, 4);
	memcpy(buffer + 21+ 16, pkg.src_IP, 33);
	memcpy(buffer + 54+ 16, pkg.dest_IP, 33);
	memcpy(buffer + 87+ 16 ,pkg.length, 11);
	memcpy(buffer + 98+ 16, pkg.data, atoi(pkg.length) + 1);
	memcpy(buffer + 99+ 16 + atoi(pkg.length), pkg.packet_life, 5);
	memcpy(buffer + 104 + 16+ atoi(pkg.length), pkg.checksum, 33);

	printPacketDEBUG("Outgoing Data",buffer);
	*/
	
	strncpy(outgoing.RouterID, pkg.router_ID, 32);
	strncpy(outgoing.PacketType, pkg.packet_type, 4);
	Trans_Data data;
	strncpy(data.src_ip, pkg.src_IP, 32);
	strncpy(data.des_ip, pkg.dest_IP, 32);
	data.length = atoi(pkg.length);
	memcpy(data.data, pkg.data, atoi(pkg.length) + 1);
	
	outgoing.Data = data;
	memcpy(outgoing.PacketChecksum, pkg.checksum, 32);
	return outgoing;
}

struct packet convertCharToPacket(Packet buffer)
{
	struct packet pkg;
	//OLD CODE
	/*
	memcpy(pkg.router_ID, buffer, 17);	
	memcpy(pkg.packet_type, buffer + 17, 4);
	memcpy(pkg.src_IP, buffer + 21, 33);
	memcpy(pkg.dest_IP, buffer + 54 ,33);
	memcpy(pkg.length, buffer + 87, 11);
	pkg.data = malloc( atoi(pkg.length) + 1);
	memcpy(pkg.data, buffer + 98, atoi(pkg.length) + 1);
	memcpy(pkg.packet_life,buffer + 99 + atoi(pkg.length), 5);
	memcpy(pkg.checksum,buffer + 104 + atoi(pkg.length), 33);

	printPacketDEBUG("Incoming Data",buffer);
	free(buffer);
	*/
	
	strncpy(pkg.router_ID, buffer.RouterID, 32);
	strncpy(pkg.packet_type, buffer.PacketType, 4);
	Trans_Data data;
	data = buffer.Data;
	strncpy(pkg.src_IP, data.src_ip, 32);
	strncpy(pkg.dest_IP, data.des_ip, 32);
	snprintf(pkg.length, 11, "%010d", data.length);
	pkg.data = malloc( atoi(pkg.length) + 1);
	memcpy(pkg.data, data.data, atoi(pkg.length) + 1);
	memcpy(pkg.checksum, buffer.PacketChecksum, 32);

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

    FILE * f;
    char filename[80];
    strcpy(filename, "../.");
    strcat(filename, addrstr);
    strcat(filename, "\0");
    if(f = fopen(filename, "w"))
    {
        fputs(str,f);
	 fclose(f);
    }
    else
        printf("ERROR: Writing port to file\n");

    Listen(sockfd, 1024);
    printf("STARTUP: Server is setup on port %d\n", port);
    if(fork() == 0)
	{
    while(1)
	{
    		client_fd = Accept(sockfd, client_sockaddr, sin_size);

    		/* Receive from the remote side */
		int cont = 1;
		while(cont == 1)
		{
    		Packet recieved;
    		Recv(client_fd,&recieved,sizeof(recieved),0);
		struct packet pkt = convertCharToPacket(recieved);
		if(lsrp_incomingmessage(pkt) == 0)
		{
			Packet ack = convertPacketToChar(lsrp_createACK(pkt));
			Send(client_fd, &ack, sizeof(ack),0);
    			//close(client_fd); // do not close client socket 
			//cont = 0;
		}
		}
	}
	}
}

void socket_sendFile(char * hostname, int port, struct packet pkg)
{
    int sockfd,sendbytes,recvbytes;
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
    int pid = 0;
    int cont = 1;
    while(cont == 1)
    {
    if((pid = fork()) == 0)
    { 
	Packet toSend = convertPacketToChar(pkg);
        sendbytes = Send(sockfd,&toSend,sizeof(toSend), 0);
	Packet recv;
        Recv(sockfd,&recv,sizeof(recv), 0); 
        _exit(0);
    }
    else
    {
	int i = 0;
	while( i < timeout*10)
	{
	int status;
	if(waitpid(pid, &status, WNOHANG)  > 0){
		cont = 0;
		break;
	}
	usleep(100); //timeout
	i++;
	}
	if(cont == 1)
	{
	printf("#ERROR: Timeout occurred. Resending packet. \n");
	kill(pid, SIGKILL);
	}
	
    }
    }
    close(sockfd);
    free(pkg.data);
}
