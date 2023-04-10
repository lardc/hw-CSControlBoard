#include "Measurement.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "SysConfig.h"
#include "ZwADC.h"

float MEASURE_GetPressureValue()
{
	float Offset, K, Pressure, P2, P1;
	int16_t P0;

	Offset = (float)((int16_t)DataTable[REG_PRESSURE_OFFSET]);
	K = (float)DataTable[REG_PRESSURE_K] / 1000;
	Pressure = ((float)ADC_Measure(ADC1, ADC_PRESSURE_CHANNEL) - Offset) * ADC_REF_VOLTAGE / ADC_RESOLUTION * K;

	// ������ ���������� ��������� ����������
	P2 = (float)((int16_t) DataTable[REG_PRESSURE_P2]) / 1000000;
	P1 = (float)DataTable[REG_PRESSURE_P1] / 1000;
	P0 = (int16_t) DataTable[REG_PRESSURE_P0];

	Pressure = Pressure * Pressure * P2 + Pressure * P1 + P0;

	return (Pressure > 0) ? Pressure : 0;
}
//------------------------------------------------
