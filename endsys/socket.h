#ifndef socketHEADER
#define socketHEADER
#include "lsrp.h"
#include "commonItems.h"
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h> 

	void socket_rcvFile();
	int socket_getIP();

#endif
