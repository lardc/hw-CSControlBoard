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
void ZbGPIO_Init();
void ZbGPIO_CSMux(Int16U SPIDevice);

inline Boolean ZbGPIO_PressureOK()
{
	return TRUE;
}
// ----------------------------------------

inline Boolean ZbGPIO_GetS3State()
{
	return TRUE;
}
// ----------------------------------------

inline void ZbGPIO_EnablePowerSwitch(Boolean State)
{
}
// ----------------------------------------

inline void ZbGPIO_PneumoPushUp(Boolean State)
{
}
// ----------------------------------------

inline void ZbGPIO_PneumoPushOut(Boolean State)
{
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
