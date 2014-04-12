#include "commonItems.h"
int MTU = 1000;
char edge_IP[32] = {'0','0','0','.','0','0','0','.','0','0','0','.','0','0','0','\0'};
int edge_Port = 0;
int timeout = 100;
int errorRate = 0;

void commonItems_setMTU(int MTU_temp)
{
	MTU = MTU_temp;
}

void commonItems_setEdgeRouter(int port, char IP[16])
{
	edge_Port = port;
	memcpy(edge_IP, IP, sizeof(edge_IP));
}

void commonItems_setTimeout(int time)
{
	timeout = time;
}

void commonItems_displaysettings()
{
	printf("Timeout is set to: %d\n", timeout);
	printf("MTU is set to: %d\n", MTU);
	printf("Edge router is: %s@%d\n", edge_IP, edge_Port);
	printf("Error rate is %d\n", errorRate);
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
