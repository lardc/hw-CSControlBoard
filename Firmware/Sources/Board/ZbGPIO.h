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
	return !ZwGPIO_ReadPin(PIN_SEN2);
}
// ----------------------------------------

inline Boolean ZbGPIO_B1Pushed()
{
	return ZwGPIO_ReadPin(PIN_SEN1);
}
// ----------------------------------------

inline Boolean ZbGPIO_B2Pushed()
{
	return ZwGPIO_ReadPin(PIN_SEN2);
}
// ----------------------------------------

inline Boolean ZbGPIO_GetS3State()
{
	return ZwGPIO_ReadPin(PIN_SEN3);
}
// ----------------------------------------

inline void ZbGPIO_HeadsUp(Boolean State)
{
	ZwGPIO_WritePin(PIN_D5D6, State);
}
// ----------------------------------------

inline void ZbGPIO_OpenBeerValve(Boolean State)
{
	ZwGPIO_WritePin(PIN_D3D4, State);
	ZwGPIO_WritePin(PIN_D7D8, State);
}
// ----------------------------------------

inline void ZbGPIO_OpenCO2Valve(Boolean State)
{
	ZwGPIO_WritePin(PIN_FAN, State);
}
// ----------------------------------------

inline void ZbGPIO_EnablePowerSwitch(Boolean State)
{
}
// ----------------------------------------

inline void ZbGPIO_PneumoPushUp(Boolean State)
{
	ZwGPIO_WritePin(PIN_M1M2, State);
}
// ----------------------------------------

inline void ZbGPIO_PneumoPushOut(Boolean State)
{
	ZwGPIO_WritePin(PIN_M3M4, State);
}
// ----------------------------------------

inline void ZbGPIO_SwitchFan(Boolean State)
{
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

#endif // __ZBGPIO_H
