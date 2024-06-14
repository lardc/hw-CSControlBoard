// Includes
#include "Board.h"
#include "Logic.h"
#include "DataTable.h"
#include "LowLevel.h"
#include "Math.h"
#include "Delay.h"
#include "Controller.h"
#include "Constraints.h"

// Functions prototypes
bool LOGIC_IDVoltageInRane(float IDVoltage, Int16U ReferenceReg);

// Functions
//
DUTType LOGIC_AdapterIDMatch(float IDVoltage)
{
	if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MCDA))
		return DT_MCDA;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIAA))
		return DT_MIAA;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIDA))
		return DT_MIDA;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIFA))
		return DT_MIFA;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIHA))
		return DT_MIHA;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIHM))
		return DT_MIHM;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIHV))
		return DT_MIHV;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MISM))
		return DT_MISM;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MISM2))
		return DT_MISM2;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MISV))
		return DT_MISV;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIXM))
		return DT_MIXM;
	else if(LOGIC_IDVoltageInRane(IDVoltage, REG_ADPTR_REF_MIXV))
		return DT_MIXV;
	else
		return DT_None;
}
//------------------------------------------

bool LOGIC_IDVoltageInRane(float IDVoltage, Int16U ReferenceReg)
{
	return(fabsf(IDVoltage - DataTable[ReferenceReg])) < VOLTAGE_ID_ABSOLUTE_MAX_ERR ||
			(fabsf(IDVoltage - DataTable[ReferenceReg]) / DataTable[ReferenceReg]) < VOLTAGE_ID_RELATIVE_MAX_ERR;
}
//------------------------------------------

void LOGIC_UpdateSensors()
{
	DataTable[REG_TL_DUT_PRESENCE] = LL_GetStatePresenceSensorDUT1() ? YES : NO;
	DataTable[REG_TR_DUT_PRESENCE] = LL_GetStatePresenceSensorDUT2() ? YES : NO;
	DataTable[REG_BL_DUT_PRESENCE] = LL_GetStatePresenceSensorDUT3() ? YES : NO;
	DataTable[REG_BR_DUT_PRESENCE] = LL_GetStatePresenceSensorDUT4() ? YES : NO;

	DataTable[REG_SEN_TOP_ADAPTER] = LL_GetStateLimitSwitchTopAdapter() ? YES : NO;
	DataTable[REG_SEN_BOT_ADAPTER] = LL_GetStateLimitSwitchBotAdapter() ? YES : NO;
}
//------------------------------------------
