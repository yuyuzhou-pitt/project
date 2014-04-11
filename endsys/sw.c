#include "sw.h"

void sw_readInCfg()
{
   	FILE *file = fopen ( "sw.cfg", "r" );
   	if ( file != NULL )
   	{
      		char line[128];
      		while ( fgets ( line, sizeof(line), file) != NULL )
      		{
			if(line[0]!='#')
			{
        			if(strstr(line,"timeout"))
				{
					char *n;
					n = strtok(line, " ");
					n = strtok(NULL, " ");
					int n_val = atoi(n);
					commonItems_setTimeout(n_val);
				}
			}
      		}
      		fclose (file);
  	}
   	else
   	{
      		printf("Could not open sw.cfg");
   	}
}

int sw_getAppID()
{
	int pid = getpid();
	return pid;
}

struct data_segment segment_encapsulation(struct message msg, int size)
{
	struct data_segment ds;
	strcpy(ds.packet_type,"0");
	snprintf(ds.app_id, sizeof(ds.app_id), "%016d", sw_getAppID());
	strcpy(ds.sequence_number, "001\0");
	
	ds.data = malloc(size);
	memset(ds.data, 0, size); 
	memcpy(ds.data, msg.app_id, 17);
	memcpy(ds.data + 17, msg.length, 11);
	memcpy(ds.data + 28, msg.end_flag, 2);
	memcpy(ds.data + 30, msg.data, atoi(msg.length) + 1);
	memcpy(ds.data + 30 + atoi(msg.length) + 1, msg.checksum, 33);
	free(msg.data);
	return ds;
}

struct message segment_decapsulation(struct data_segment ds)
{
	struct message msg;
	strcpy(msg.app_id, ds.data);
	strcpy(msg.length, ds.data + 17);
	strcpy(msg.end_flag, ds.data + 28);
	msg.data = malloc(atoi(msg.length) + 1);
	memset(msg.data, 0, atoi(msg.length) + 1); 
	strcpy(msg.data, ds.data + 30);
	strcpy(msg.checksum, ds.data + 30 + atoi(msg.length) + 1);
	free(ds.data);

	return msg;
}

void sw_outgoingmessage(struct message msg, int size)
{
	struct data_segment ds = segment_encapsulation(msg, size);
	lsrp_outgoingmessage(ds, size + 23);
}

void sw_incomingmessage(struct data_segment ds)
{
	struct message msg = segment_decapsulation(ds);
	app_incomingFile(msg);
}

