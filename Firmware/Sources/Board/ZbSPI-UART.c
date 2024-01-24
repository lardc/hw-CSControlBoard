// Header
#include "ZbSPI-UART.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"


// Variables
static volatile Boolean SU_RxFIFOEmpty = TRUE;
static volatile Boolean SU_TxFIFOEmpty = TRUE;
static volatile Int64U	CounterValue;

// Functions
//
Int16U ZbSUe_ReadWrite(Int16U Data)
{
	Int16U InputData;

	// Set proper SPI settings
	ZwSPIa_Init(TRUE, SPIA_BAUDRATE, 16, SPIA_PLR, SPIA_PHASE, 0, TRUE, FALSE);
	ZbGPIO_CSMux(SPIMUX_SU);

	// Data exchange
	ZwSPIa_BeginReceive(&Data, 1, 16, STTNormal);
	while(ZwSPIa_GetWordsToReceive() == 0);
	ZwSPIa_EndReceive(&InputData, 1);

	return InputData;
}
// ----------------------------------------

void ZbSUe_Init()
{
	Int16U config = 0;

	config |= 1u << 15;		// required
	config |= 1u << 14;		// required
	config |= 0u << 13;		// FIFO is enabled
	config |= 0u << 12;		// no software shutdown
	config |= 0u << 11;		// no nTM mask
	config |= 0u << 10;		// no nRM mask
	config |= 0u <<  9;		// no nPM mask
	config |= 0u <<  8;		// no nRAM mask
	config |= 0u <<  7;		// no IR timing
	config |= 0u <<  6;		// one stop bit
	config |= 0u <<  5;		// no parity bit
	config |= 0u <<  4;		// 8bit message

	// baud rate
	// 115200
	config |= 0u <<  3;		// B3
	config |= 0u <<  2;		// B2
	config |= 0u <<  1;		// B1
	config |= 0u <<  0;		// B0

	ZbSUe_ReadWrite(config);
}
// ----------------------------------------

void ZbSUe_FIFOStatusUpdate()
{
	Int16U config = ZbSUe_ReadWrite(0x4000);

	SU_RxFIFOEmpty = (config & BIT15) == 0 ? TRUE : FALSE;
	SU_TxFIFOEmpty = (config & BIT14) != 0 ? TRUE : FALSE;
}
// ----------------------------------------

void ZbSUe_SendData(pInt16U Buffer, Int16U BufferSize)
{
	Int16U TxCounter = 0;

	// Switch to TX mode
	ZwGPIO_WritePin(PIN_RS485_CTRL, TRUE);

	// Send data
	while (TxCounter < BufferSize)
	{
		do
		{
			ZbSUe_FIFOStatusUpdate();
		}
		while(!SU_TxFIFOEmpty);

		ZbSUe_ReadWrite((Buffer[TxCounter] & 0xFF) | 0x8000);
		TxCounter++;
	}

	// Switch to RX mode
	do
	{
		ZbSUe_FIFOStatusUpdate();
	}
	while(!SU_TxFIFOEmpty);
	DELAY_US(100);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
}
// ----------------------------------------

Int16S ZbSUe_ReadData(pInt16U Buffer, Int16U BufferSize)
{
	Int16U Char;
	Int16U RxCounter = 0;
	Int64U CounterCopy = CounterValue;

	// Frame start
	do
	{
		// Wait for input data
		do
		{
			ZbSUe_FIFOStatusUpdate();
			if (CounterValue - CounterCopy > TRM_TIMEOUT_TICKS) return -1;
		}
		while(SU_RxFIFOEmpty);

		Char = 0xFF & ZbSUe_ReadWrite(0x00);
	}
	while (Char != 0x23);

	// Read frame body
	Buffer[RxCounter++] = Char;
	do
	{
		// Wait for input data
		do
		{
			ZbSUe_FIFOStatusUpdate();
			if (CounterValue - CounterCopy > TRM_TIMEOUT_TICKS) return -1;
		}
		while(SU_RxFIFOEmpty);

		Char = 0xFF & ZbSUe_ReadWrite(0x00);
		Buffer[RxCounter++] = Char;
	}
	while (Char != 0x0D && RxCounter < BufferSize);

	if (Char == 0x0D)
		return RxCounter;
	else
		return 0;
}
// ----------------------------------------

void ZbSUe_UpdateTimeCounter(Int64U Counter)
{
	CounterValue = Counter;
}
// ----------------------------------------
