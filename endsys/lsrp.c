#include "lsrp.h"

void packet_causeError(struct packet* pkt)
{
	srand(time(NULL));
	int prob = rand() % 100;
	
	if(errorRate > prob)
	{
		int r = rand() % atoi((*pkt).length);
		((unsigned char*)(*pkt).data)[r] = '1';
	}
}

struct packet lsrp_createACK(struct packet recv)
{
	struct packet pck;
	memset(pck.router_ID,0,32);
	memset(pck.packet_type,0,3);
	memset(pck.dest_IP,0,32);
	memset(pck.src_IP,0,32);
	memset(pck.packet_life,'1',4);
	memset(pck.length,0,10);

	strncpy(pck.src_IP, recv.dest_IP, strlen(recv.dest_IP));
	strncpy(pck.dest_IP, recv.src_IP, strlen(recv.src_IP));
	strncpy(pck.router_ID, recv.router_ID, strlen(recv.router_ID));
	memcpy(pck.packet_type, "110\0", 4); // use different type for ACK
	memcpy(pck.packet_life + 4 , "\0", 1);
	
	struct data_segment ds = sw_getACK();
	pck.data = malloc(23);
	memset(pck.data, 0, 23);
	memcpy(pck.data, ds.packet_type, 2);
	memcpy(pck.data + 2, ds.app_id , 17);
	memcpy(pck.data + 19, ds.sequence_number, 4);
	
	
	snprintf(pck.length, 11, "%010d", 23);
	snprintf(pck.checksum, sizeof(pck.checksum), "%032d",chksum_crc32((unsigned char*) pck.data, atoi(pck.length)));

	return pck;
}

struct packet packet_encapsulation(struct data_segment ds, int size,  char * IP)
{
	struct packet pck;
	memset(pck.router_ID,0,32);
	memset(pck.packet_type,0,3);
	memset(pck.dest_IP,0,32);
	memset(pck.src_IP,0,32);
	memset(pck.packet_life,'1',4);
	memset(pck.length,0,10);

	char src_ip[32];
	socket_getIP(src_ip);
	strncpy(pck.src_IP, src_ip, strlen(src_ip));
	strncpy(pck.dest_IP, IP, strlen(IP));
	strncpy(pck.router_ID, src_ip, strlen(src_ip)); // to tell the remote where this packet comes from
	memcpy(pck.packet_type, "100\0", 4);
	memcpy(pck.packet_life + 4 , "\0", 1);
	
	pck.data = malloc(size + 23);
	memset(pck.data, 0, size);
	memcpy(pck.data, ds.packet_type, 2);
	memcpy(pck.data + 2, ds.app_id , 17);
	memcpy(pck.data + 19, ds.sequence_number, 4);
	memcpy(pck.data + 23, ds.data, size);
	free(ds.data);
	
	snprintf(pck.length, 11, "%010d", size);
	snprintf(pck.checksum, sizeof(pck.checksum), "%032d",chksum_crc32((unsigned char*) pck.data, atoi(pck.length)));
	return pck;

}

struct data_segment packet_decapsulation(struct packet pkt)
{
	struct data_segment ds;
	strcpy(ds.packet_type, pkt.data);
	strcpy(ds.app_id, pkt.data + 2);
	strcpy(ds.sequence_number, pkt.data + 19);
	int  n = atoi(pkt.length) + 1;
	ds.data = malloc(n);
	memset(ds.data, 0, n);
	memcpy(ds.data, pkt.data + 23, n);
	free(pkt.data);
	return ds;
}

void lsrp_outgoingmessage(struct data_segment ds, int size, char * IP)
{
	struct packet pck = packet_encapsulation(ds, size, IP);
	packet_causeError(&pck);

	if(edge_Port == -1)
	{
		char *portstr = malloc(6);
		FILE * f;
		char filename[80];
    		strcpy(filename, "../.");
    		strcat(filename, edge_IP);
		strcat(filename, "\0");
		if((f = fopen(filename, "r")) != NULL)
		{
			fseek(f, 0, SEEK_SET);
			if(fgets(portstr, 6, f) != NULL)
			{
				edge_Port = atoi(portstr);
				fclose(f);
			}
			else
			{
				fclose(f);
				printf("ERROR: Could not read port number \n");
				exit(-1);
			}
		}
		else
		{
			printf("ERROR: Could not open port number file\n");
			exit(-1);
		}
	}
	if(edge_Port == -1) //Check to see if it is still -1. Means there is an error
	{
		printf("ERROR: Could not determine Port number from IP %s",edge_IP);
		exit(0);
	}
	socket_sendFile(edge_IP, edge_Port, pck);
}

int lsrp_incomingmessage(struct packet pkt)
{
	if(commonfunctions_checkCRC_pkt(pkt) != 0)
	{
		printf("ERROR: CRC not correct at LSRP\n");
		return -1;
	}
	else
	{
		struct data_segment ds = packet_decapsulation(pkt);
		return sw_incomingmessage(ds);
	}
}

void lsrp_readInCfg()
{
	FILE *file = fopen ( "lsrp.cfg", "r" );
   	if ( file != NULL )
   	{
      		char line[128];
      		while ( fgets ( line, sizeof(line), file) != NULL )
      		{
			if(line[0]!='#')
			{
        			if(strstr(line,"packet_error_rate") != NULL)
				{
					char *n;
					n = strtok(line, "=");
					n = strtok(NULL, "=");
					int packetError = atoi(n);
					commonItems_setErrorRate(packetError);
				}
				if(strstr(line,"ech0_direct_link_addr") != NULL)
				{
					line[strlen(line)-1] = 0;
					char IP_address[32];
					memcpy(IP_address, line + 22, sizeof(IP_address));
					commonItems_setEdgeIP(IP_address);
				}
			}
      		}
      		fclose (file);
  	}
   	else
   	{
      		printf("Could not open lsrp.cfg");
   	}
}
