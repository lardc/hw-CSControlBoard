// Header
#include "DebugActions.h"

// Include
//
#include "LowLevel.h"
#include "Board.h"
#include "Delay.h"
#include "Controller.h"
#include "DataTable.h"

// Functions
// Send pulse to "CSM Block" indicator
void DBACT_ToggleIndCSM()
{
	LL_SetStateIndCSM(true);
	DELAY_MS(1000);
	LL_SetStateIndCSM(false);
}
//-----------------------

// Send pulse to "Adapter Block" indicator
void DBACT_ToggleIndADPTR()
{
	LL_SetStateIndADPTR(true);
	DELAY_MS(1000);
	LL_SetStateIndADPTR(false);
}
//-----------------------

// Send pulse to Top adapter blocking pneumatic cylinder
void DBACT_TogglePneumTOP()
{
	LL_SetStatePneumTOP(true);
	DELAY_MS(1000);
	LL_SetStatePneumTOP(false);
}
//-----------------------

// Send pulse to Bottom adapter blocking pneumatic cylinder
void DBACT_TogglePneumBOT()
{
	LL_SetStatePneumBOT(true);
	DELAY_MS(1000);
	LL_SetStatePneumBOT(false);
}
//-----------------------

// Send pulse to DUT blocking pneumatic cylinder
void DBACT_TogglePneumDUT()
{
	LL_SetStatePneumDUT(true);
	DELAY_MS(1000);
	LL_SetStatePneumDUT(false);
}
//-----------------------

// Send pulse to Safety Subsystem Output
void DBACT_ToggleSFOutput()
{
	LL_SetStateSFOutput(true);
	DELAY_MS(1000);
	LL_SetStateSFOutput(false);
}
//-----------------------

// Measure voltage at the resistor divider on the top side of the adapter
void DBACT_MeasureIDDividerTop()
{
	DataTable[REG_RSLT] = LL_MeasureIDTop();
}
//-----------------------

// Measure voltage at the resistor divider on the bottom side of the adapter
void DBACT_MeasureIDDividerBot()
{
	DataTable[REG_RSLT] = LL_MeasureIDBot();
}
//-----------------------

// Measure voltage from the pressure sensor
void DBACT_MeasurePressure()
{
	DataTable[REG_RSLT] = LL_MeasurePressure();
}
//-----------------------
