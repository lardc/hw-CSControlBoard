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

inline Boolean ZbGPIO_IsSafetySensorOk()
{
	return !ZwGPIO_ReadPin(PIN_SAFETY);
}
// ----------------------------------------

inline Boolean ZbGPIO_HomeSensorActuate()
{
	return ZwGPIO_ReadPin(PIN_HOME);
}
// ----------------------------------------

inline Boolean ZbGPIO_IsBusToolingSensorOk()
{
	return ZwGPIO_ReadPin(PIN_SEN1);
}
// ----------------------------------------

inline Boolean ZbGPIO_IsAdapterToolingSensorOk()
{
	return !ZwGPIO_ReadPin(PIN_SEN2);
}
// ----------------------------------------

inline void ZbGPIO_SwitchPowerConnection(Boolean State)
{
	ZwGPIO_WritePin(PIN_OUT3, State);
}
// ----------------------------------------

inline Boolean ZbGPIO_IsPowerConnected()
{
	return ZwGPIO_ReadPin(PIN_OUT3);
}
// ----------------------------------------

inline void ZbGPIO_SwitchControlConnection(Boolean State)
{
	ZwGPIO_WritePin(PIN_OUT4, State);
}
// ----------------------------------------

inline Boolean ZbGPIO_IsControlConnected()
{
	return ZwGPIO_ReadPin(PIN_OUT4);
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

inline void ZbGPIO_SwitchUpDir(Boolean State)
{
	ZwGPIO_WritePin(PIN_STPM_DIR, !State);
}
// ----------------------------------------

inline Boolean ZbGPIO_IsDirUp()
{
	return !ZwGPIO_ReadPin(PIN_STPM_DIR);
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
