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

struct data_segment segment_encapsulation(struct message msg)
{
	struct data_segment ds;
	ds.packet_type ='0';
	snprintf(ds.app_id, sizeof(ds.app_id), "%d", sw_getAppID());
	ds.sequence_number[0] ='0';
	ds.sequence_number[1] ='0';
	ds.sequence_number[2] ='1';
	ds.data = &msg;
	return ds;
}

struct message segment_decapsulation(struct data_segment ds)
{
	return *(ds.data);
}

void sw_outgoingmessage(struct message msg)
{
	struct data_segment ds = segment_encapsulation(msg);
	lsrp_outgoingmessage(ds);
}

void sw_incomingmessage(struct data_segment ds)
{
	struct message msg = segment_decapsulation(ds);
	app_incomingFile(msg);
}

