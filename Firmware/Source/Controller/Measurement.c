#include "Measurement.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "SysConfig.h"
#include "ZwADC.h"

float MEASURE_GetPressureValue()
{
	float Offset, K, Pressure, P2, P1, P0;

	Offset = DataTable[REG_PRESSURE_OFFSET];
	K = DataTable[REG_PRESSURE_K];
	Pressure = ((float)ADC_Measure(ADC2, ADC_PRESSURE_CHANNEL) - Offset) * ADC_REF_VOLTAGE / ADC_RESOLUTION * K;

	// Тонкая подстройка измерения напряжения
	P2 = DataTable[REG_PRESSURE_P2];
	P1 = DataTable[REG_PRESSURE_P1];
	P0 = DataTable[REG_PRESSURE_P0];

	Pressure = Pressure * Pressure * P2 + Pressure * P1 + P0;

	return (Pressure > 0) ? Pressure : 0;
}
//------------------------------------------------
