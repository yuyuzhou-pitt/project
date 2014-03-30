#ifndef appHEADER
#define appHEADER
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "sw.h"
#include "commonItems.h"
	void app_sendFile(char filename[FILENAMESIZE], char DEST_FILE[160], char * IP);
	void app_getFile(struct message msg);
	void app_readInCfg();
#endif
