// Header
#include "TemperatureFeedback.h"
//
// Includes
#include "stdlib.h"
#include "AnalogOutput.h"
#include "ZbThermistor.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "SysConfig.h"
#include "ZwUtils.h"

// Definitions
#define TEMPFB_AVG_NUM		4
#define TEMPFB_SPIKES_NUM	2
#define TEMPFB_BUFFER_SIZE	(TEMPFB_AVG_NUM + (TEMPFB_SPIKES_NUM * 2))

// Variables
static Int64U TempFb_TimeCounterPrevValue = 0;
static _iq TempFb_SamplesCh1[TEMPFB_BUFFER_SIZE] = {0};
static _iq TempFb_SamplesCh2[TEMPFB_BUFFER_SIZE] = {0};
static _iq TempFb_SamplesCopy[TEMPFB_BUFFER_SIZE] = {0};

// Functions
//
_iq TempFb_GetTemperatureCH1()
{
	return _IQmpyI32(_FPtoIQ2(DataTable[REG_TEMP_READ_K_N], DataTable[REG_TEMP_READ_K_D]), ZbTh_ReadSEN1());
}
// ----------------------------------------

_iq TempFb_GetTemperatureCH2()
{
	return _IQmpyI32(_FPtoIQ2(DataTable[REG_TEMP_READ_K_N], DataTable[REG_TEMP_READ_K_D]), ZbTh_ReadSEN2());
}
// ----------------------------------------

int inline SortCompareVals(const void *A, const void *B)
{
	return *((_iq *)A) - *((_iq *)B);
}
// ----------------------------------------

_iq TempFb_ExtractChannelData()
{
	Int16U i;
	_iq Avg = 0;

	qsort(TempFb_SamplesCopy, TEMPFB_BUFFER_SIZE, sizeof(_iq), &SortCompareVals);

	// Calculate average
	for (i = TEMPFB_SPIKES_NUM; i < (TEMPFB_SPIKES_NUM + TEMPFB_AVG_NUM); i++)
		Avg += TempFb_SamplesCopy[i];

	return _IQdiv(Avg, _IQI(TEMPFB_AVG_NUM));
}
// ----------------------------------------

void TempFb_UpdateTemperatureFeedback(Int64U TimeCounter)
{
	_iq Temp1, Temp2;
	Int16U i, TimeDelta = TimeCounter - TempFb_TimeCounterPrevValue;

	if (TimeDelta >= TEMPFB_CONV_TICKS)
	{
		TempFb_TimeCounterPrevValue = TimeCounter;

		// Data input
		Temp1 = TempFb_GetTemperatureCH1();
		Temp2 = TempFb_GetTemperatureCH2();

		// Shift buffer
		for (i = 0; i < (TEMPFB_BUFFER_SIZE - 1); i++)
		{
			TempFb_SamplesCh1[i] = TempFb_SamplesCh1[i + 1];
			TempFb_SamplesCh2[i] = TempFb_SamplesCh2[i + 1];
		}

		TempFb_SamplesCh1[TEMPFB_BUFFER_SIZE - 1] = Temp1;
		TempFb_SamplesCh2[TEMPFB_BUFFER_SIZE - 1] = Temp2;
		
		// Reject spikes
		MemCopy32((pInt32U)TempFb_SamplesCh1, (pInt32U)TempFb_SamplesCopy, TEMPFB_BUFFER_SIZE);
		Temp1 = TempFb_ExtractChannelData();
		MemCopy32((pInt32U)TempFb_SamplesCh2, (pInt32U)TempFb_SamplesCopy, TEMPFB_BUFFER_SIZE);
		Temp2 = TempFb_ExtractChannelData();

		// Data output
		AnalogOutput_SetCH1(Temp1);
		DataTable[REG_TEMP_CH1] = _IQmpyI32int(Temp1, 10);
		AnalogOutput_SetCH2(Temp2);
		DataTable[REG_TEMP_CH2] = _IQmpyI32int(Temp2, 10);
	}
}
// ----------------------------------------

// No more.
