#ifndef checksumHEADER
#define checksumHEADER	
#include <stdio.h>
#include <stdlib.h>

unsigned int chksum_crc32 (unsigned char *block, unsigned int length);
void chksum_crc32gentab();
#endif
