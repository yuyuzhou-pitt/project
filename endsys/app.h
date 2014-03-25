#ifndef appHEADER
#define appHEADER
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "sw.h"
#include "commonItems.h"
	void app_sendFile( char IP[80], char filename[FILENAMESIZE]);
	void app_getFile(struct message msg);
#endif
