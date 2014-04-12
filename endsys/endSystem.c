#include <stdio.h>
#include <string.h>
#include "app.h"
#include "sw.h"
#include "commonItems.h"
#include "socket.h"

void readcfgs()
{
	app_readInCfg();
	sw_readInCfg();
	lsrp_readInCfg();
}

int main( int argc, const char* argv[] )
{
	chksum_crc32gentab();
	printf("STARTUP: CRC has been setup\n");

	socket_printIP();
	socket_rcvFile();
	readcfgs();
	printf("STARTUP: Config files have been read\n");
	
	char cmd[80];
	printf("#");
	scanf ("%79s",cmd);  
	while(strcmp(cmd,"exit") != 0)
	{
		if(strstr(cmd,"sendfile"))
		{
			char filename[FILENAMESIZE];
			char *IP; char *FILE_DEST;
			char IP_FILE[160];

			scanf ("%79s",filename); 
			scanf ("%159s", IP_FILE);
			if(commonfunctions_checkSetRouter() == -1)
				printf("ERROR: Edge router not set\n");
			else
			{
				IP = strtok(IP_FILE, ":");
				FILE_DEST = strtok(NULL, ":");
				app_outgoingFile( filename, FILE_DEST, IP);
			}
			//exit(0);
		}
		else if(strstr(cmd,"set-edge-router"))
		{
			int port = 0;
			char IP[80];
			scanf ("%79s",IP);  
			scanf ("%d",&port);
			
			char IP_address[32];
			memcpy(IP_address, IP, sizeof(IP_address));
			commonItems_setEdgeRouter(port,IP_address);
		}
		else if(strstr(cmd,"MTU"))
		{
			int MTU_temp = 0;
			scanf ("%d",&MTU_temp);   
			commonItems_setMTU(MTU_temp);
			printf("MTU is set to: %d bytes\n", MTU);	
		}	
		else if(strcmp(cmd,"printSettings") == 0)
		{
			commonItems_displaysettings();
		}
		else if(strcmp(cmd,"rereadConfig") == 0)
		{
			readcfgs();
			printf("STATUS: Config files have been reread\n");
		}
		else if(strcmp(cmd,"test") == 0)
		{  
			struct packet pkg;
			pkg.data = malloc(10);
			socket_sendFile(edge_IP,edge_Port, pkg);		
		}
		else if(strcmp(cmd,"help") == 0)
		{
			printf("The following commands are available:\n");
			printf("sendfile <filepath> <dst_ip>:<dst_path>\n");	
			printf("MTU <size>\n");	
			printf("set-edge-router <IP> <Port>\n");
			printf("printSettings\n");
			printf("rereadConfig\n");			
		}
		else		
		{
			printf("The command was not recognized.\n");
			printf("Please try \"help\" a for list of commands\n");		
		}
		
		printf("#");
		scanf ("%79s",cmd);  
	}
	exit(0);
}
