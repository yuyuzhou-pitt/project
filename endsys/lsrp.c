#include "lsrp.h"

struct packet packet_encapsulation(struct data_segment ds)
{
	struct packet pck;
	pck.data = &ds;
	snprintf(pck.length, sizeof(pck.length), "%d", sizeof(pck.data));
	snprintf(pck.checksum, sizeof(pck.checksum), "%d",chksum_crc32((unsigned char*) pck.data, atoi(pck.length)));
	return pck;
}

struct data_segment packet_decapsulation(struct packet pkt)
{
	if(commonfunctions_checkCRC_pkt(pkt) != 0)
	{
		//TODO
		printf("ERROR: CRC not correct at LSRP\n");
	} 
	return *(pkt.data);
}

void lsrp_outgoingmessage(struct data_segment ds)
{
	struct packet pck = packet_encapsulation(ds);
	lsrp_incomingmessage(pck);
}

void lsrp_incomingmessage(struct packet pkt)
{
	struct data_segment ds = packet_decapsulation(pkt);
	sw_incomingmessage(ds);
}
