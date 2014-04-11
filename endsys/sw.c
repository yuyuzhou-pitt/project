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
	ds.sequence_number[0] ='0';
	ds.sequence_number[1] ='0';
	ds.sequence_number[2] ='1';
	ds.sequence_number[3] ='\0';
	
	ds.data = malloc(size);
	memset(ds.data, 0, size); 
	memcpy(ds.data, msg.app_id, 17);
printf("Msg msg.app_id: %s", msg.app_id);
	printf("Msg id: %s", msg.app_id);
	memcpy(ds.data + 17, msg.length,11);
	printf("Msg length: %s", msg.length);
	memcpy(ds.data + 28, msg.end_flag,2);
printf("Msg end: %s", msg.end_flag);
	memcpy(ds.data + 30, msg.data, atoi(msg.length)+ 1 );
	printf("Msg data: %s\n", ds.data);
fflush(stdout);
	memcpy(ds.data +30 + atoi(msg.length) + 1, msg.checksum,33);
	free(msg.data);
printf("test");
fflush(stdout);
	return ds;
}

struct message segment_decapsulation(struct data_segment ds)
{
	printf("\n\n\n%s\n\n\n",ds.data);
fflush(stdout);
	struct message msg;
	strcpy(msg.app_id, ds.data);
	printf("Msg id: %s", msg.app_id);
printf("Msg msg.app_id: %s", msg.app_id);
	strcpy(msg.length, ds.data + 17);
	printf("Msg length: %s", msg.length);
	strcpy(msg.end_flag, ds.data + 28);
	printf("Msg end: %s", msg.end_flag);
	msg.data = malloc(atoi(msg.length) + 1);
	memset(msg.data, 0, atoi(msg.length) + 1); 
	strcpy(msg.data, ds.data + 30);
	printf("Msg data: %s", msg.data);
	strcpy(msg.checksum, ds.data + 30 + atoi(msg.length) + 1);
	free(ds.data);

	return msg;
}

void sw_outgoingmessage(struct message msg, int size)
{
	struct data_segment ds = segment_encapsulation(msg, size);
	lsrp_outgoingmessage(ds, size + 23);
	//sw_incomingmessage(ds);
}

void sw_incomingmessage(struct data_segment ds)
{
	struct message msg = segment_decapsulation(ds);
	app_incomingFile(msg);
}

