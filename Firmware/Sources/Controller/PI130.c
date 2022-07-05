// Header
#include "PI130.h"

// Include
#include "Controller.h"
#include "ZbBoard.h"

// Definitions
#define BYTES_READ_TIMEOUT		100		// in ms

// Forward functions
Int16U PI130_Crc(pInt16U data_value, Int16U length);

// Functions
//
Int16U PI130_Crc(pInt16U data_value, Int16U length)
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

void PI130_StartMotor(Boolean State)
{
	Int16U Char = 0;
	Int16U Data[8] = {0x01, 0x06, 0x20, 0x00, 0x00, 0x01, 0, 0};

	Int16U CRC = PI130_Crc(Data, 6);
	Data[6] = CRC >> 8;
	Data[7] = CRC & 0xff;
	ZbSU_SendData(Data, 8);

	Int64U LastByteRead = CONTROL_TimeCounter;
	while((LastByteRead + BYTES_READ_TIMEOUT) > CONTROL_TimeCounter && CONTROL_Rx_Counter < VALUES_RX_SIZE)
	{
		if(ZbSU_Read(&Char))
		{
			CONTROL_RS485_Rx[CONTROL_Rx_Counter++] = Char;
			LastByteRead = CONTROL_TimeCounter;
		}
	}
}
