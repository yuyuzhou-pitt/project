#include "app.h"

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

struct message message_encapsulation(struct metadata MD, int frag)
{
	FILE *fp;
	char line[80];
	struct message message;
	message.data = (char *)malloc(maxDataSize);
	fp = fopen(MD.name, "r");
	fseek(fp, frag*maxDataSize, SEEK_SET);

	int maxLeft = maxDataSize;
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
	return message;
}

void message_decapsulation(struct message msg, int frag)
{
	FILE *fp;
	fp = fopen("test2.txt", "w");
	fseek(fp, frag*maxDataSize, SEEK_SET);
	fputs(msg.data, fp);
	fclose(fp);
}

void app_sendFile(char IP[80], char filename[FILENAMESIZE])
{
	struct metadata file;
	file.length = getFileSize(filename);
	strcpy(file.name, filename);
	file.fragments = (file.length + maxDataSize- 1)/maxDataSize;
	
	for(int i = 0; i < file.fragments; i++)
	{
		printf("test");
		struct message temp_message = message_encapsulation(file, i);
		//CALL SW function
		//test
		message_decapsulation(temp_message, i);
	}
}

void app_getFile(struct message msg)
{
	
}
 