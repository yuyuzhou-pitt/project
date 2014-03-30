#ifndef commonItemsHEADER
#define commonItemsHEADER	
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "checksum.h"

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

	struct packet
	{
		char router_ID[16];
		char packet_type[3]; // data packets are 110
		char src_IP[32];
		char dest_IP[32];
		char length[10];
		struct data_segment* data; 
		char packet_life[4];
		char checksum[32]; 
	};


	void commonItems_setMTU(int MTU_temp);
	void commonItems_setEdgeRouter(int port, char IP[15]);
	void commonItems_setTimeout(int time);
	void commonItems_displaysettings();
	int commonfunctions_checkCRC(struct message msg);

	extern int MTU;
	extern int edge_Port;
	extern char edge_IP[16];
	extern int timeout;
#endif
