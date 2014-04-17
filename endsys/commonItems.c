#include "commonItems.h"
int MTU = 1000;
char edge_IP[17] = {'0','0','0','.','0','0','0','.','0','0','0','.','0','0','0','\0'};
int edge_Port = 0;
int timeout = 100;
int errorRate = 0;
int DEBUG = 0;

void commonItems_setMTU(int MTU_temp)
{
	MTU = MTU_temp;
}

void commonItems_setEdgeRouter(int port, char IP[17])
{
	edge_Port = port;
	memcpy(edge_IP, IP, sizeof(edge_IP));
}

void commonItems_setTimeout(int time)
{
	timeout = time;
}

void commonItems_setDEBUG(int debug_val)
{
	DEBUG = debug_val;
}

void commonItems_displaysettings()
{
	printf("Timeout is set to: %d\n", timeout);
	printf("MTU is set to: %d\n", MTU);
	printf("Edge router is: %s@%d\n", edge_IP, edge_Port);
	printf("Error rate is %d\n", errorRate);
	printf("Debug is set to %d\n", DEBUG);
}

int commonfunctions_checkCRC(struct message msg)
{
	unsigned int generated = chksum_crc32((unsigned char*) msg.data, atoi(msg.length));
	unsigned int given = atoi(msg.checksum);

	if(generated != given)
		return -1;
	else
		return 0;
}

int commonfunctions_checkCRC_pkt(struct packet pck)
{
	unsigned int generated = chksum_crc32((unsigned char *) pck.data, atoi(pck.length));
	unsigned int given = atoi(pck.checksum);	

	if(generated != given)
		return -1;
	else
		return 0;
}

int commonfunctions_checkSetRouter()
{
	if(edge_IP == "000.000.000.000" || edge_Port == 0)
		return -1;
	else
		return 0;
}

void commonItems_setErrorRate(int ER)
{
	errorRate = ER;
}




//DEBUG
void printDEBUG(char* string)
{
	if(DEBUG == 1)
		printf("DEBUG: %s\n",string);
}

void printPacketDEBUG(char* outOrIn, char* string)
{
	if(DEBUG == 1)
	{
		printf("DEBUG: %s\n",outOrIn);
		printf("ROUTER ID: %.*s\n", 17, string);
		printf("PACKET TYPE: %.*s\n",4, string + 17);
		printf("SRC IP: %.*s\n",33, string + 21);
		printf("DEST IP: %.*s\n",33, string + 54);
		printf("LENGTH: %.*s\n",11, string + 87);
	
		char * length = malloc(11);
		memcpy(length, string + 87, 11);

		fwrite(string + 98, 1,atoi(length) + 1, stdout);
		printf("\nPACKET_LIFE: %.*s\n",5, string +  99 + atoi(length));
		printf("CHECKSUM: %.*s\n",33, string + 104 + atoi(length));
		printf("\n\n\n");
		free(length);
	}
}

