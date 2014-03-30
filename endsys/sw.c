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

int getAppID()
{
	int pid = getpid();
	return pid;
}

struct data_segment segment_encapsulation(struct message msg)
{
	struct data_segment ds;
}

struct message segment_decapsulation(struct data_segment ds)
{

}
