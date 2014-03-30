#include <stdio.h>
#include <string.h>
#include "app.h"
#include "sw.h"
#include "commonItems.h"

void readcfgs()
{
	app_readInCfg();
	sw_readInCfg();
}

int main( int argc, const char* argv[] )
{
	readcfgs();
	printf("STARTUP: Config files have been read\n");

	chksum_crc32gentab();
	printf("STARTUP: CRC has been setup\n");
	
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
			IP = strtok(IP_FILE, ":");
			FILE_DEST = strtok(NULL, ":");
			app_sendFile( filename, FILE_DEST, IP);
		}
		else if(strstr(cmd,"set-edge-router"))
		{
			int port = 0;
			char IP[80];
			scanf ("%79s",IP);  
			scanf ("%d",&port);
			
			char IP_address[16];
			memcpy(IP_address, IP, sizeof(IP_address));
			commonItems_setEdgeRouter(port,IP_address);
			printf("Server has been set up on this machine. \nPlease ensure all servers are setup. \nThen press 1 to continue to setup client:");
			
			int cont = 0;
			scanf ("%d",&cont);
			if(cont == 1)
			{
				//CALL SETUP CLIENT
			}
			else
			{
				printf("You have choosen not to continue. Server will be shutdown");
			}
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
}
