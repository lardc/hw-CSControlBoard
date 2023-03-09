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
	ZwGPIO_WritePin(PIN_DUT_TAIL_MP, FALSE);
	ZwGPIO_WritePin(PIN_DUT_BODY_MP, FALSE);
	ZwGPIO_WritePin(PIN_DUT_TAIL_PP, FALSE);
	ZwGPIO_WritePin(PIN_DUT_BODY_PP, FALSE);
	ZwGPIO_WritePin(PIN_BTN_TAIL_LED, FALSE);
	ZwGPIO_WritePin(PIN_BTN_BODY_LED, FALSE);
	ZwGPIO_WritePin(PIN_FAN, FALSE);
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
	ZwGPIO_WritePin(PIN_SPIMUX_A, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_B, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_C, TRUE);
	ZwGPIO_WritePin(PIN_AOUT_LDAC, TRUE);
   	// Configure pins
   	ZwGPIO_PinToOutput(PIN_WD_RST);
   	ZwGPIO_PinToOutput(PIN_DUT_BODY_PP);
   	ZwGPIO_PinToOutput(PIN_DUT_TAIL_PP);
   	ZwGPIO_PinToOutput(PIN_DUT_BODY_MP);
   	ZwGPIO_PinToOutput(PIN_DUT_TAIL_MP);
   	ZwGPIO_PinToOutput(PIN_BTN_TAIL_LED);
   	ZwGPIO_PinToOutput(PIN_BTN_BODY_LED);
   	ZwGPIO_PinToOutput(PIN_FAN);
   	ZwGPIO_PinToOutput(PIN_LED);
   	ZwGPIO_PinToOutput(PIN_SPIMUX_A);
   	ZwGPIO_PinToOutput(PIN_SPIMUX_B);
   	ZwGPIO_PinToOutput(PIN_SPIMUX_C);
   	ZwGPIO_PinToOutput(PIN_RS485_CTRL);
   	ZwGPIO_PinToOutput(PIN_AOUT_LDAC);

   	// Input pins
   	ZwGPIO_PinToInput(PIN_BTN_TAIL, TRUE, PQ_Sample6);
   	ZwGPIO_PinToInput(PIN_SEN2, TRUE, PQ_Sample6);
   	ZwGPIO_PinToInput(PIN_SEN3, TRUE, PQ_Sample6);
   	ZwGPIO_PinToInput(PIN_BTN_BODY, TRUE, PQ_Sample6);
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
	ZwGPIO_WritePin(PIN_SPIMUX_A, SPIDevice & BIT0);
	ZwGPIO_WritePin(PIN_SPIMUX_B, SPIDevice & BIT1);
	ZwGPIO_WritePin(PIN_SPIMUX_C, SPIDevice & BIT2);
	DELAY_US(1);
}
// ----------------------------------------

// No more.
