// Header
#include "AnalogOutput.h"
//
// Includes
#include "ZbDAC.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

// Functions
//
void AnalogOutput_SetCH1(_iq TempVal)
{
	// Main conversion
	_iq result = _IQmpy(TempVal, _FPtoIQ2(DataTable[REG_TEMP_TO_DAC_K_N], DataTable[REG_TEMP_TO_DAC_K_D]));

	ZbDAC_WriteA((result > 0) ? _IQint(result) : 0);
}
// ----------------------------------------

void AnalogOutput_SetCH2(_iq TempVal)
{
	// Main conversion
	_iq result = _IQmpy(TempVal, _FPtoIQ2(DataTable[REG_TEMP_TO_DAC_K_N], DataTable[REG_TEMP_TO_DAC_K_D]));

	ZbDAC_WriteB((result > 0) ? _IQint(result) : 0);
}
// ----------------------------------------

// No more.
