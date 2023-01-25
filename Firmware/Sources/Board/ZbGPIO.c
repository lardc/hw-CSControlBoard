// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

// Header
#include "Global.h"
#include "ZbGPIO.h"
#include "SysConfig.h"

// Variables
static Int16U SafetyCircuitCounter = 0;

// Functions
//
void ZbGPIO_Init()
{
	// Output pins
	// Reset to default state
	ZwGPIO_WritePin(PIN_WD_RST, FALSE);
	ZwGPIO_WritePin(PIN_M1M2, FALSE);
	ZwGPIO_WritePin(PIN_M3M4, FALSE);
	ZwGPIO_WritePin(PIN_SAFETY_HOLD, FALSE);
	ZwGPIO_WritePin(PIN_FAN, FALSE);
	ZwGPIO_WritePin(PIN_POWER_SWITCH, FALSE);
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
	ZwGPIO_WritePin(PIN_SPIMUX_OPT2, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_OPT1, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_AOUT, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_SU, TRUE);
	ZwGPIO_WritePin(PIN_AOUT_LDAC, TRUE);
	// Configure pins
	ZwGPIO_PinToOutput(PIN_WD_RST);
	ZwGPIO_PinToOutput(PIN_M1M2);
	ZwGPIO_PinToOutput(PIN_M3M4);
	ZwGPIO_PinToOutput(PIN_SAFETY_HOLD);
	ZwGPIO_PinToOutput(PIN_FAN);
	ZwGPIO_PinToOutput(PIN_POWER_SWITCH);
	ZwGPIO_PinToOutput(PIN_LED);
	ZwGPIO_PinToOutput(PIN_SPIMUX_OPT2);
	ZwGPIO_PinToOutput(PIN_SPIMUX_OPT1);
	ZwGPIO_PinToOutput(PIN_SPIMUX_AOUT);
	ZwGPIO_PinToOutput(PIN_SPIMUX_SU);
	ZwGPIO_PinToOutput(PIN_RS485_CTRL);
	ZwGPIO_PinToOutput(PIN_AOUT_LDAC);

	// Input pins
	ZwGPIO_PinToInput(PIN_SEN1, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_SEN2, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_SEN3, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_SAFETY, TRUE, PQ_Sample6);
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(ZbGPIO_FilterSafetyCircuit, "ramfuncs");
#endif
Boolean ZbGPIO_FilterSafetyCircuit(Boolean NewState)
{
	if(!NewState)
		SafetyCircuitCounter = 0;
	else
		SafetyCircuitCounter++;

	if(SafetyCircuitCounter > SC_FILTER_T)
		SafetyCircuitCounter = SC_FILTER_T;

	return (SafetyCircuitCounter >= SC_FILTER_T);
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
