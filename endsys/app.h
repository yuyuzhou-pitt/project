#ifndef appHEADER
#define appHEADER
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "checksum.h"
#include "sw.h"
#include "commonItems.h"

	void message_decapsulation(struct metadata MD, struct message msg, int frag);
	struct message message_encapsulation(struct metadata MD, int frag);
	void message_decapsulation(struct metadata MD, struct message msg, int frag);
	struct metadata message_decapsulation_first(struct message msg);
	
	void app_outgoingFile(char filename[FILENAMESIZE], char DEST_FILE[160], char * IP);
	void app_incomingFile(struct message msg);
	void app_readInCfg();
#endif
