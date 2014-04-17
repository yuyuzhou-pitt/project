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
		char app_id[17]; // app process id, 0~32767
		char length[11];
		char end_flag[2];
		char* data;
		char checksum[33]; 
	};

	struct data_segment
	{
		char packet_type[2]; //0-data 1-ack
		// App id number in case there are more than one file transfer in the same terminal 
		// use file transfer app process id to distinguish between different apps
		char app_id[17]; // app process id, 0~32767
		char sequence_number[4];  // 0~7
		char* data; 
	};

	struct packet
	{
		char router_ID[17];
		char packet_type[4]; // data packets are 110
		char src_IP[33];
		char dest_IP[33];
		char length[11];
		char* data; 
		char packet_life[5];
		char checksum[33]; 
	};


	void commonItems_setMTU(int MTU_temp);
	void commonItems_setEdgeRouter(int port, char IP[15]);
	void commonItems_setTimeout(int time);
	void commonItems_displaysettings();
	int commonfunctions_checkCRC(struct message msg);
	int commonfunctions_checkCRC_pkt(struct packet pck);
	int commonfunctions_checkSetRouter();
	void commonItems_setErrorRate(int errorRate);
	void commonItems_setDEBUG(int debug_val);

	extern int MTU;
	extern int edge_Port;
	extern char edge_IP[17];
	extern int timeout;
	extern int errorRate;
	extern int DEBUG;


//DEBUG
	void printPacketDEBUG(char* outOrIn, char* string);
	void printDEBUG(char* string);

#endif
