#include <stdio.h>
#include <string.h>
#include "app.h"
#include "commonItems.h"

int main( int argc, const char* argv[] )
{
	char cmd[80];
	printf("#");
	scanf ("%79s",cmd);  
	while(strcmp(cmd,"exit") != 0)
	{
		if(strstr(cmd,"sendfile"))
		{
			char filename[FILENAMESIZE];
			char IP[80];

			scanf ("%79s",IP);
			scanf ("%79s",filename); 
			app_sendFile(IP, filename);
		}
		else if(strstr(cmd,"set-edge-router"))
		{
			int port = 0;
			char IP[80];
			scanf ("%79s",IP);  
			scanf ("%d",&port);  
			//TODO
		}
		else if(strstr(cmd,"MTU"))
		{
			int MTU_temp = 0;
			scanf ("%d",&MTU_temp);   
			commonItems_setMTU(MTU_temp);
			printf("MTU is set to: %d bytes\n", MTU);	
		}	
		else if(strcmp(cmd,"help") == 0)
		{
			printf("The following commands are available:\n");
			printf("sendfile <dst_ip> <filename>\n");	
			printf("MTU <size>\n");	
			printf("set-edge-router <IP> <Port>\n");		
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
