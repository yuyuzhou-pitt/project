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

int app_getAppID()
{
	int pid = getpid();
	return pid;
}

struct message message_encapsulation_first(char* DEST_FILE)
{
	struct message message;
	message.data = (char *)malloc(strlen(DEST_FILE) + 1);
	strcpy(message.data, DEST_FILE);
	memcpy(message.data + strlen(DEST_FILE),"\0",1);
	strcpy(message.end_flag,"0");
	snprintf(message.app_id, 17, "%016d", app_getAppID());
	snprintf(message.length, 11, "%010d", strlen(message.data));
	snprintf(message.checksum, 33, "%032d",chksum_crc32((unsigned char*) message.data, strlen(message.data)));
	return message;
}

struct message message_encapsulation(struct metadata MD, int dataPos)
{
	FILE *fp;
	char line[81];
	struct message message;
	message.data = (char *)malloc(MTU + 1);
  	memset(message.data, 0, MTU + 1);
	fp = fopen(MD.name, "r");
	fseek(fp, dataPos, SEEK_SET);

	int maxLeft = MTU + 1;
	int lineLength = 81;
	if(maxLeft < 81)
		lineLength = maxLeft;
	
	while(fgets(line, lineLength, fp) != NULL && maxLeft != 0)
   	{
		strncat(message.data, line, strlen(line));
		maxLeft -= strlen(line);
		if(maxLeft < 81)
			lineLength = maxLeft + 1;
   	}
	fclose(fp);
	strcat(message.data, "\0");
	
	strcpy(message.end_flag,"0");
	snprintf(message.app_id, sizeof(message.app_id), "%016d", app_getAppID());
	snprintf(message.length, sizeof(message.length), "%010d", strlen(message.data));
	snprintf(message.checksum, sizeof(message.checksum), "%032d",chksum_crc32((unsigned char*) message.data, strlen(message.data)));
	return message;
}

void message_decapsulation(struct metadata MD, struct message msg, int pos)
{
	FILE *fp;
	fp = fopen(MD.name, "a");
	fseek(fp, pos, SEEK_SET);
	fputs(msg.data, fp);
	fclose(fp);
}

struct metadata message_decapsulation_first(struct message msg)
{
	struct metadata file;
	strcpy(file.name, msg.data);
	FILE *fp;
	fp = fopen(file.name, "w");
	fclose(fp);
	return file;
}

int sent_data = 0;

void app_outgoingFile(char filename[FILENAMESIZE], char* DEST_FILE, char * IP)
{
	struct metadata file;
	file.length = getFileSize(filename);
	strcpy(file.name, filename);
	file.fragments = (file.length + MTU- 1)/MTU;

	struct message msg1 = message_encapsulation_first(DEST_FILE);	
	sw_outgoingmessage(msg1, atoi(msg1.length) + 63 + 1, IP);
	int i = 0;
	for(; i < file.fragments; i++)
	{
		struct message temp_message = message_encapsulation(file, sent_data);
		if(i + 1 == file.fragments)
			strcpy(temp_message.end_flag,"1");
		sw_outgoingmessage(temp_message, atoi(temp_message.length) + 63 + 1, IP);	
		sent_data +=atoi(temp_message.length);
	}
	sent_data = 0;
	printf("File Sent: %s\n", file.name);
}

struct metadata* recieved = NULL;
int recieved_data = 0;

int app_incomingFile(struct message msg)
{
	if(commonfunctions_checkCRC(msg) != 0)
	{
		printf("ERROR: CRC not correct \n");
		return -1;
	}
	if( recieved == NULL)
	{
		recieved = malloc(sizeof(struct metadata));
		*recieved = message_decapsulation_first(msg);
		free(msg.data);
	}
	else
	{
		message_decapsulation(*recieved, msg, recieved_data);
		recieved_data += atoi(msg.length);
		if(msg.end_flag[0] == '1')
		{
			printf("File Recieved: %s\n", (*recieved).name);
			free(recieved);
			recieved = NULL;
			recieved_data = 0;
		}
		free(msg.data);
	}
	return 0;
}
