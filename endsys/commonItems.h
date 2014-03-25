#ifndef commonItemsHEADER
#define commonItemsHEADER	

#define FILENAMESIZE 80

	struct metadata
	{
		unsigned long length;
		char name[FILENAMESIZE];
		int fragments;
	};

	struct message
	{
		char app_id[16]; // app process id, 0~32767
		char length[10];
		char end_flag[1];
		char* data;
		char checksum[32]; 
	};

	struct data_segment
	{
		char packet_type; //0-data 1-ack
		// App id number in case there are more than one file transfer in the same terminal 
		// use file transfer app process id to distinguish between different apps
		char app_id[16]; // app process id, 0~32767
		char sequence_number[3];  // 0~7
		struct message* data; 
	};

	void commonItems_setMTU(int MTU_temp);
	
	extern int MTU;
	extern int maxDataSize;
#endif
