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

void socket_rcvFile()
{
struct	sockaddr_in local,remote;
int	rlen=sizeof(remote);
int sk, len=sizeof(local),rsk;

/* Create an internet domain stream socket */
sk=socket(AF_INET,SOCK_STREAM,0);

/*Construct and bind the name using default values*/
local.sin_family=AF_INET;
local.sin_addr.s_addr=INADDR_ANY;
local.sin_port=0;
bind(sk,(struct sockaddr *)&local,sizeof(local));

/*Find out and publish socket name */
getsockname(sk,(struct sockaddr *)&local,&len);
if(fork()==0)
{
	printf("STARTUP: Server is setup on port %d\n", local.sin_port);
	/* Start accepting connections */
	/*Declare willingness to accept a connection*/
	listen(sk,5); 
	while(1) 
	{
  		rsk=accept(sk,0,0);/*Accept new request for a connection*/

  		if(fork()==0) /*Create one child to serve each client*/
   		{
			//recvfrom(sk,pkt,atoi(pkt.length),0,(struct sockaddr *)&remote,&rlen);
    			//Read in struct
   		}
  		else
    			close(rsk);
  	}
}
}

int socket_getIP()
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
