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
	_iq result, tmp2 = _IQdiv(TempVal, _IQ(1000.0f));

	// Fine tuning
	result = _IQmpy(tmp2, _IQmpyI32(tmp2, (Int16S)DataTable[REG_TEMP1_TO_DAC_FINE_P2])) + _IQmpy(TempVal, _FPtoIQ2(DataTable[REG_TEMP1_TO_DAC_FINE_P1], 1000)) +
			_FPtoIQ2((Int16S)DataTable[REG_TEMP1_TO_DAC_FINE_P0], 10);

	// Main conversion
	result = _IQmpy(result, _FPtoIQ2(DataTable[REG_TEMP_TO_DAC_K_N], DataTable[REG_TEMP_TO_DAC_K_D]));

	ZbDAC_WriteA((result > 0) ? _IQint(result) : 0);
}
// ----------------------------------------

void AnalogOutput_SetCH2(_iq TempVal)
{
	_iq result, tmp2 = _IQdiv(TempVal, _IQ(1000.0f));

	// Fine tuning
	result = _IQmpy(tmp2, _IQmpyI32(tmp2, (Int16S)DataTable[REG_TEMP2_TO_DAC_FINE_P2])) + _IQmpy(TempVal, _FPtoIQ2(DataTable[REG_TEMP2_TO_DAC_FINE_P1], 1000)) +
			_FPtoIQ2((Int16S)DataTable[REG_TEMP2_TO_DAC_FINE_P0], 10);

	// Main conversion
	result = _IQmpy(result, _FPtoIQ2(DataTable[REG_TEMP_TO_DAC_K_N], DataTable[REG_TEMP_TO_DAC_K_D]));

	ZbDAC_WriteB((result > 0) ? _IQint(result) : 0);
}
// ----------------------------------------

// No more.
