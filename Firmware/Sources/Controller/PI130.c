// Header
#include "PI130.h"

// Functions
//
unsigned int PI130_Crc(unsigned char *data_value, unsigned char length)
{
	unsigned int crc_value = 0xFFFF;
	int i;
	while(length--)
	{
		crc_value ^= *data_value++;
		for(i = 0; i < 8; i++)
		{
			if(crc_value & 0x0001)
			{
				crc_value = (crc_value >> 1) ^ 0xa001;
			}
			else
			{
				crc_value = crc_value >> 1;
			}
		}
	}
	return crc_value;
}
