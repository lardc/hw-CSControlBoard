// Header
#include "ZbDAC.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"

// Forward functions
//
static void ZbDAC_WriteX(Int16U Data, Boolean UseChannelB);

// Functions
//
void ZbDAC_WriteA(Int16U Data)
{
	ZbDAC_WriteX(Data, FALSE);
}
// ----------------------------------------

void ZbDAC_WriteB(Int16U Data)
{
	ZbDAC_WriteX(Data, TRUE);
}
// ----------------------------------------

static void ZbDAC_WriteX(Int16U Data, Boolean UseChannelB)
{
	Int16U dummy;

	// Set proper SPI settings
	ZwSPIa_Init(TRUE, SPIA_BAUDRATE, 16, SPIA_PLR, SPIA_PHASE, 0, TRUE, FALSE);
	ZbGPIO_CSMux(SPIMUX_AOUT);

	// Check for saturation
	if (Data > 0xFFF) Data = 0xFFF;

	// Set proper channel
	if (UseChannelB) Data |= BIT15;
	// Set buffering
	Data |= BIT14;

	// Send data
	ZwSPIa_Send(&Data, 1, 16, STTNormal);
	// Wait for the end of transmission
	while(ZwSPIa_GetWordsToReceive() == 0);
	ZwSPIa_EndReceive(&dummy, 1);
	
	// Strobe to latch
	ZwGPIO_WritePin(PIN_AOUT_LDAC, FALSE);
    asm(" RPT #2 || NOP");
	ZwGPIO_WritePin(PIN_AOUT_LDAC, TRUE);
}
// ----------------------------------------

// No more.
