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

inline Boolean ZbGPIO_GetS3State()
{
	return ZwGPIO_ReadPin(PIN_SEN3);
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
	ZwGPIO_WritePin(PIN_FAN, State);
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED()
{
	ZwGPIO_TogglePin(PIN_LED);
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED_Tail()
{
	ZwGPIO_TogglePin(PIN_BTN_TAIL_LED);
}
// ----------------------------------------

inline void ZbGPIO_LEDTail(Boolean State)
{
	ZwGPIO_WritePin(PIN_BTN_TAIL_LED, State);
}
// ----------------------------------------

inline void ZbGPIO_ToggleLED_Body()
{
	ZwGPIO_TogglePin(PIN_BTN_BODY_LED);
}
// ----------------------------------------

inline void ZbGPIO_LEDBody(Boolean State)
{
	ZwGPIO_WritePin(PIN_BTN_BODY_LED, State);
}
// ----------------------------------------

inline Boolean ZbGPIO_BTN_DUT_State(Int16U PinNumber)
{
	return !ZwGPIO_ReadPin(PinNumber);
}
// ----------------------------------------

inline void ZbGPIO_DUT_MainPinchControl(Boolean State)
{
	ZwGPIO_WritePin(PIN_DUT_TAIL_MP, State);
	ZwGPIO_WritePin(PIN_DUT_BODY_MP, State);
}
// ----------------------------------------

inline void ZbGPIO_DUT_TailControl(Boolean State)
{
	ZwGPIO_WritePin(PIN_DUT_TAIL_PP, State);
}
// ----------------------------------------

inline void ZbGPIO_DUT_BodyControl(Boolean State)
{
	ZwGPIO_WritePin(PIN_DUT_BODY_PP, State);
}
// ----------------------------------------

#endif // __ZBGPIO_H
