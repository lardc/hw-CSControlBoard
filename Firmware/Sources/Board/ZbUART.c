// Header
#include "ZbUART.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"


// Variables
static volatile Int64U	CounterValue;

// Functions
//
void ZbSU_Write(Int16U Data)
{
	// Data exchange
	ZwSCIa_SendChar(Data);
}
// ----------------------------------------

Boolean ZbSU_Read(pInt16U Byte)
{
	if(ZwSCIa_GetBytesToReceive() != 0)
	{
		*Byte = ZwSCIa_ReceiveChar() & 0xFF;
		return TRUE;
	}
	else
		return FALSE;
}
// ----------------------------------------

void ZbSU_SendData(pInt16U Buffer, Int16U BufferSize)
{
	Int16U TxCounter = 0;

	// Switch to TX mode
	ZwGPIO_WritePin(PIN_RS485_CTRL, TRUE);

	// Send data
	while (TxCounter < BufferSize)
	{
		ZbSU_Write(Buffer[TxCounter] & 0xFF);
		TxCounter++;
	}

	// Switch to RX mode
	DELAY_US(BufferSize * 100);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
}
// ----------------------------------------

Int16S ZbSU_ReadData(pInt16U Buffer, Int16U BufferSize)
{
	Int16U Char = 0;
	Int16U RxCounter = 0;
	Int64U CounterCopy = CounterValue;

	// Frame start
	do
	{
		// Wait for input data
		if (CounterValue - CounterCopy > TRM_TIMEOUT_TICKS) return -1;

		 ZbSU_Read(&Char);
	}
	while (Char != 0x23);

	// Read frame body
	Buffer[RxCounter++] = Char;
	do
	{
		// Wait for input data
		if (CounterValue - CounterCopy > TRM_TIMEOUT_TICKS) return -1;

		if(ZbSU_Read(&Char))
			Buffer[RxCounter++] = Char;
	}
	while (Char != 0x0D && RxCounter < BufferSize);

	if (Char == 0x0D)
		return RxCounter;
	else
		return 0;
}
// ----------------------------------------

void ZbSU_UpdateTimeCounter(Int64U Counter)
{
	CounterValue = Counter;
}
// ----------------------------------------
// No more.
