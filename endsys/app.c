#include "app.h"

void app_readInCfg()
{
   	FILE *file = fopen ( "app.cfg", "r" );
   	if ( file != NULL )
   	{
      		char line[128];
      		while ( fgets ( line, sizeof(line), file) != NULL )
      		{
			if(line[0]!='#')
			{
        			if(strstr(line,"edge-router"))
				{
					char *IP, *port;
					IP = strtok(line, " ");
					IP = strtok(NULL, " ");
					port = strtok(NULL, " ");
					int port_num = atoi(port);
					commonItems_setEdgeRouter(port_num, IP);
				}
				if(strstr(line,"MTU"))
				{
					char *n;
					n = strtok(line, " ");
					n = strtok(NULL, " ");					
					int a = atoi(n);
					commonItems_setMTU(a);
				}
			}
      		}
      		fclose (file);
  	}
   	else
   	{
      		printf("Could not open app.cfg");
   	}
}

unsigned long getFileSize(char filename[FILENAMESIZE])
{
	FILE * f;
	if(f = fopen(filename, "r"))
	{
		fseek(f, 0, SEEK_END);
		unsigned long len = (unsigned long)ftell(f);
		fclose(f);
		return len;
	}
	else
		return 0;
}

int getAppID()
{
	int pid = getpid();
	return pid;
}

struct message message_encapsulation_first(char* DEST_FILE)
{
	struct message message;
	message.data = (char *)malloc(strlen(DEST_FILE));
	message.data = DEST_FILE;
	message.end_flag[0] = '0';
	snprintf(message.app_id, sizeof(message.app_id), "%d", getAppID());
	snprintf(message.length, sizeof(message.length), "%d", strlen(message.data));
	return message;
}

struct message message_encapsulation(struct metadata MD, int frag)
{
	FILE *fp;
	char line[80];
	struct message message;
	message.data = (char *)malloc(MTU);
	fp = fopen(MD.name, "r");
	fseek(fp, frag*MTU, SEEK_SET);

	int maxLeft = MTU;
	int lineLength = 80;
	if(maxLeft < 80)
		lineLength = maxLeft;
	
	while(fgets(line, lineLength, fp) != NULL && maxLeft != 0)
   	{
		strcat(message.data, line);
		maxLeft -= lineLength;
		if(maxLeft < 80)
			lineLength = maxLeft;
   	}
	fclose(fp);

	message.end_flag[0] = '0';
	snprintf(message.app_id, sizeof(message.app_id), "%d", getAppID());
	snprintf(message.length, sizeof(message.length), "%d", strlen(message.data));
	return message;
}

void message_decapsulation(struct metadata MD, struct message msg, int frag)
{
	FILE *fp;
	fp = fopen(MD.name, "w");
	fseek(fp, frag*MTU, SEEK_SET);
	fputs(msg.data, fp);
	fclose(fp);
}

struct metadata message_decapsulation_first(struct message msg)
{
	struct metadata file;
	strcpy(file.name, msg.data);
	return file;
}

void app_sendFile(char filename[FILENAMESIZE], char* DEST_FILE, char * IP)
{
	struct metadata file;
	file.length = getFileSize(filename);
	strcpy(file.name, filename);
	file.fragments = (file.length + MTU- 1)/MTU;

	struct message msg1 = message_encapsulation_first(DEST_FILE);
	//CALL SW function	
	app_getFile(msg1);


	for(int i = 0; i < file.fragments; i++)
	{
		struct message temp_message = message_encapsulation(file, i);
		if(i + 1 == file.fragments)
			temp_message.end_flag[0] = '1';
		segment_encapsulation(temp_message);
		//test
		app_getFile(temp_message);	
	}
}

struct metadata* recieved = NULL;
int recieved_pos = 0;

void app_getFile(struct message msg)
{
	if( recieved == NULL)
	{
		recieved = malloc(sizeof(struct metadata));
		*recieved = message_decapsulation_first(msg);
	}
	else
	{
		message_decapsulation(*recieved, msg, recieved_pos);
		recieved_pos++;
		if(msg.end_flag[0] == '1')
		{
			free(recieved);
			recieved_pos = 0;
			recieved = NULL;
		}
	}
	free(msg.data);
}
