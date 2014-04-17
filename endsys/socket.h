#ifndef socketHEADER
#define socketHEADER
#include "lsrp.h"
#include "commonItems.h"
#include <strings.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h> 
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "libsocket.h"

	void socket_rcvFile();
	int socket_printIP();
	char* socket_getIP();
	void socket_sendFile(char * hostname, int port, struct packet pkg);

#endif
