// Header
#include "ZbFan.h"
//
#include "SysConfig.h"
#include "BoardConfig.h"

// Functions
//
void ZbFan_Init()
{
	ZwPWM1_Init(PWMUp, CPU_FRQ, ZW_PWM_FREQUENCY, TRUE, FALSE, 0, 0, TRUE, TRUE, TRUE, FALSE, FALSE);
	ZbFan_SetSilent(TRUE);
	ZwPWM_Enable(TRUE);
}
// ----------------------------------------

void ZbFan_SetSilent(Boolean Enable)
{
	if (Enable)
		ZwPWM1_SetValueA(((ZW_PWM_DUTY_BASE >> 1) * FAN_SILENT_DUTY) / 100);
	else
		ZwPWM1_SetValueA(((ZW_PWM_DUTY_BASE >> 1) * FAN_PERFORM_DUTY) / 100);
}
// ----------------------------------------

// No more.
