// Header
#include "ZbThermistor.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"

// Functions
//
Int16U ZbTh_ReadSENx()
{
	Int16U Data = 0;

	// Read thermocouple data
	ZwSPIb_BeginReceive(NULL, 1, 16, STTNormal);
	while(ZwSPIb_GetWordsToReceive() == 0)
		DELAY_US(1);
	ZwSPIb_EndReceive(&Data, 1);

	// remove 3 leading and 1 trailing bit
	return (Data & 0x1fff) >> 1;
}
// ----------------------------------------

Int16U ZbTh_ReadSEN1()
{
	Int16U result = 0;

	ZbGPIO_CSMux(SPIMUX_OPTO1);
	result = ZbTh_ReadSENx();
	DELAY_US(1);

	return result;
}
// ----------------------------------------

Int16U ZbTh_ReadSEN2()
{
	Int16U result = 0;

	ZbGPIO_CSMux(SPIMUX_OPTO2);
	result = ZbTh_ReadSENx();
	DELAY_US(1);

	return result;
}
// ----------------------------------------

// No more.
