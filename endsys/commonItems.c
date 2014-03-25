#include "commonItems.h"
int MTU = 1518;
int maxDataSize = 1518;

void commonItems_setMTU(int MTU_temp)
{
	MTU = MTU_temp;
	maxDataSize = MTU;
}
