// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"
#include "DataTable.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_SetStateIndCSM(bool State)
{
	GPIO_SetState(GPIO_IND_CSM, State);
}
//-----------------------------

void LL_SetStateIndADPTR(bool State)
{
	GPIO_SetState(GPIO_IND_ADPTR, State);
}
//-----------------------------

void LL_SetStatePneumTOP(bool State)
{
	GPIO_SetState(GPIO_PNEUM_TOP, State);
}
//-----------------------------

void LL_SetStatePneumBOT(bool State)
{
	GPIO_SetState(GPIO_PNEUM_BOT, State);
}
//-----------------------------

void LL_SetStatePneumDUT(bool State)
{
	GPIO_SetState(GPIO_PNEUM_DUT, State);
}
//-----------------------------

void LL_SetStateSFOutput(bool State)
{
	GPIO_SetState(GPIO_SF_OUT, State);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT1()
{
	return GPIO_GetState(GPIO_DUT_1);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT2()
{
	return GPIO_GetState(GPIO_DUT_2);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT3()
{
	return GPIO_GetState(GPIO_DUT_3);
}
//-----------------------------

bool LL_GetStatePresenceSensorDUT4()
{
	return GPIO_GetState(GPIO_DUT_4);
}
//-----------------------------

bool LL_GetStateLimitSwitchTopAdapter()
{
	return GPIO_GetState(GPIO_ADPTR_TOP);
}
//-----------------------------

bool LL_GetStateLimitSwitchBotAdapter()
{
	return GPIO_GetState(GPIO_ADPTR_BOT);
}
//-----------------------------

float LL_MeasureIDTop()
{
	return (float)ADC_Measure(ADC1, ADC_ID_TOP_CHANNEL) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------

float LL_MeasureIDBot()
{
	return (float)ADC_Measure(ADC1, ADC_ID_BOT_CHANNEL) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------

float LL_MeasurePressure()
{
	return (float)ADC_Measure(ADC2, ADC_PRESSURE_CHANNEL) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------
