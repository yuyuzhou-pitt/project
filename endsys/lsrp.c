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

struct packet packet_encapsulation(struct data_segment ds, int size)
{
	struct packet pck;
	memset(pck.router_ID,'0',16);
	memset(pck.packet_type,'0',3);
	memset(pck.dest_IP,'0',32);
	memset(pck.src_IP,'0',32);
	memset(pck.packet_life,'1',4);
	memset(pck.length,'0',10);

	char * src_ip = socket_getIP();
	memcpy(pck.src_IP + 33 - strlen(src_ip) - 1, src_ip, strlen(src_ip) + 1);
	memcpy(pck.dest_IP + 33 - strlen(edge_IP) - 1, edge_IP, strlen(edge_IP) + 1);
	memcpy(pck.router_ID + 16, "\0", 1);
	memcpy(pck.packet_type, "110\0", 4);
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

void lsrp_outgoingmessage(struct data_segment ds, int size)
{
	struct packet pck = packet_encapsulation(ds, size);
	//packet_causeError(&pck);
	socket_sendFile(edge_IP, edge_Port, pck);
}

void lsrp_incomingmessage(struct packet pkt)
{
	if(commonfunctions_checkCRC_pkt(pkt) != 0)
	{
		//TODO
		//SEND NAN
		printf("ERROR: CRC not correct at LSRP\n");
	}
	else
	{
		struct data_segment ds = packet_decapsulation(pkt);
		sw_incomingmessage(ds);
		//SEND ACK
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
        			if(strstr(line,"packet_error_rate"))
				{
					char *n;
					n = strtok(line, "=");
					n = strtok(NULL, "=");
					int packetError = atoi(n);
					commonItems_setErrorRate(packetError);
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
