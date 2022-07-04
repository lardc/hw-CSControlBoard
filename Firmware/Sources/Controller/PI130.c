// Header
#include "PI130.h"

// Definitions
#define RX_BUFFER_LEN			32
#define BYTES_READ_TIMEOUT		100		// in ms

// Include
#include "Controller.h"
#include "ZbBoard.h"

// Variables
static volatile Int16U RxBuffer[RX_BUFFER_LEN] = {0};

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
	Int16U RxCounter = 0;

	while((LastByteRead + BYTES_READ_TIMEOUT) > CONTROL_TimeCounter && RxCounter < RX_BUFFER_LEN)
	{
		if(ZbSU_Read(&Char))
		{
			RxBuffer[RxCounter++] = Char;
			LastByteRead = CONTROL_TimeCounter;
		}
	}
}
