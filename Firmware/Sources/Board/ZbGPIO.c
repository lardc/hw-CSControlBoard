// ----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

// Header
#include "Global.h"
#include "ZbGPIO.h"
#include "SysConfig.h"

// Functions
//
void ZbGPIO_Init()
{
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
	ZwGPIO_WritePin(PIN_AOUT_LDAC, TRUE);

	ZwGPIO_PinToOutput(PIN_LED);
	ZwGPIO_PinToOutput(PIN_SPIMUX_OPT2);
	ZwGPIO_PinToOutput(PIN_SPIMUX_OPT1);
	ZwGPIO_PinToOutput(PIN_SPIMUX_AOUT);
	ZwGPIO_PinToOutput(PIN_SPIMUX_SU);
	ZwGPIO_PinToOutput(PIN_RS485_CTRL);
	ZwGPIO_PinToOutput(PIN_AOUT_LDAC);
}
// ----------------------------------------

void ZbGPIO_CSMux(Int16U SPIDevice)
{
	switch(SPIDevice)
	{
		case SPIMUX_AOUT:
			ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_AOUT, FALSE);
			ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
			break;

		case SPIMUX_SU:
			ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_SU, FALSE);
			break;

		case SPIMUX_OPTO1:
			ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_OPT1, FALSE);
			ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
			break;

		case SPIMUX_OPTO2:
			ZwGPIO_WritePin(PIN_SPIMUX_OPT2, FALSE);
			ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
			break;

		default:
			ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
			ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
			break;
	}
	DELAY_US(1);
}
// ----------------------------------------
