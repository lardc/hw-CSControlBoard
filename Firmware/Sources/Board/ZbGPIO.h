// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

#ifndef __ZBGPIO_H
#define __ZBGPIO_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"
#include "Global.h"

// Functions
//
// Init module
void ZbGPIO_Init();
// Get pin state
Boolean ZbGPIO_FilterSafetyCircuit(Boolean NewState);
// Set proper CS output
void ZbGPIO_CSMux(Int16U SPIDevice);

inline Boolean ZbGPIO_PressureOK()
{
	return DBG_USE_AIR_CTRL ? !ZwGPIO_ReadPin(PIN_SEN2) : TRUE;
}
// ----------------------------------------

inline Boolean ZbGPIO_GetS3State()
{
	return ZwGPIO_ReadPin(PIN_SEN3);
}
// ----------------------------------------

inline void ZbGPIO_EnablePowerSwitch(Boolean State)
{
	ZwGPIO_WritePin(PIN_POWER_SWITCH, State);
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

#endif // __ZBGPIO_H
