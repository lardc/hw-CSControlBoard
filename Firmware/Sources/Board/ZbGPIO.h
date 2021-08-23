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

inline Boolean ZbGPIO_HomeSensorActuate()
{
	return !ZwGPIO_ReadPin(PIN_SEN1);
}
// ----------------------------------------

inline Boolean ZbGPIO_GetPowerConnectionState()
{
	return ZwGPIO_ReadPin(PIN_SEN3);
}
// ----------------------------------------

inline void ZbGPIO_SwitchPowerConnection(Boolean State)
{
	ZwGPIO_WritePin(PIN_OUT3, State);
}
// ----------------------------------------

inline void ZbGPIO_ToggleStep()
{
	ZwGPIO_TogglePin(PIN_STPM_STEP);
}
// ----------------------------------------

inline void ZbGPIO_SwitchStep(Boolean State)
{
	ZwGPIO_WritePin(PIN_STPM_STEP, State);
}
// ----------------------------------------

inline void ZbGPIO_SwitchDir(Boolean State)
{
	ZwGPIO_WritePin(PIN_STPM_DIR, State);
}
// ----------------------------------------

inline void ZbGPIO_SwitchEnable(Boolean State)
{
	ZwGPIO_WritePin(PIN_STPM_EN, State);
}
// ----------------------------------------

inline void ZbGPIO_SwitchFan(Boolean State)
{
	ZwGPIO_WritePin(PIN_FAN, State);
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

#endif // __ZBGPIO_H
