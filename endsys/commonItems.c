#include "commonItems.h"
int MTU = 1000;
char edge_IP[16] = {'0','0','0','.','0','0','0','.','0','0','0','.','0','0','0','\0'};
int edge_Port = 0;
int timeout = 100;

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
}
