// Header
#include "ZbAnalogInput.h"
//
// Includes
#include "ZwDSP.h"
#include "ClampSettings.h"

// Definitions
#define DATA_ARRAY_SIZE		FORCE_AVG_SAMPLES

// Variables
static const Int16U DataAIN[16] = { ADC_INA0, ADC_INA0, ADC_INA0, ADC_INA0,\
									ADC_INA0, ADC_INA0, ADC_INA0, ADC_INA0,\
									ADC_INA0, ADC_INA0, ADC_INA0, ADC_INA0,\
									ADC_INA0, ADC_INA0, ADC_INA0, ADC_INA0 };
static Int16U AnalogInResult;
static Boolean EnableAcq = FALSE;
//
static Int16U DataArray[DATA_ARRAY_SIZE] = {0};

// Forward functions
static void ZbAnanlogInput_Acquire(Int16U * const restrict pResults);

// Functions
//
void ZbAnanlogInput_StartAcquisition()
{
	if (EnableAcq) ZwADC_StartSEQ1();
}
// ----------------------------------------

void ZbAnanlogInput_EnableAcq(Boolean Flag)
{
	EnableAcq = Flag;
}
// ----------------------------------------

void ZbAnanlogInput_Init()
{
	// Subscribe to results
	ZwADC_ConfigureSequentialCascaded(ADC_SNUM, DataAIN);
	ZwADC_SubscribeToResults1(&ZbAnanlogInput_Acquire);
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(ZbAnanlogInput_GetResult, "ramfuncs");
#endif
Int16U ZbAnanlogInput_GetResult()
{
	return AnalogInResult;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(ZbAnanlogInput_Acquire, "ramfuncs");
#endif
static void ZbAnanlogInput_Acquire(Int16U * const restrict pResults)
{
	Int16U i, tmp = 0;
	Int32U AvgSum = 0;

	// Average ADC result
	for (i = 0; i < ADC_SNUM; i++)
		tmp += pResults[i];
	tmp >>= 4;

	// Average ring buffer
	for (i = 1; i < DATA_ARRAY_SIZE; i++)
	{
		DataArray[i - 1] = DataArray[i];
		AvgSum += DataArray[i];
	}
	DataArray[DATA_ARRAY_SIZE - 1] = tmp;
	AvgSum += tmp;

	AnalogInResult = (Int16U)(AvgSum / DATA_ARRAY_SIZE);
}
// ----------------------------------------

// No more.
