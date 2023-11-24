﻿// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

// Header
#include "Global.h"
#include "ZbGPIO.h"
#include "SysConfig.h"

// Variables
static Int16U SafetyCircuitCounter = 0;

// Functions
//
void ZbGPIO_Init()
{
	// Output pins
	ZwGPIO_PinToOutput(PIN_STPM_EN);
	ZwGPIO_PinToOutput(PIN_STPM_DIR);
	ZwGPIO_PinToOutput(PIN_STPM_STEP);
	ZwGPIO_PinToOutput(PIN_OUT4);
	ZwGPIO_PinToOutput(PIN_FAN);
	ZwGPIO_PinToOutput(PIN_LED);
	ZwGPIO_PinToOutput(PIN_OUT3);
	ZwGPIO_PinToOutput(PIN_SPIMUX_A);
	ZwGPIO_PinToOutput(PIN_SPIMUX_B);
	ZwGPIO_PinToOutput(PIN_SPIMUX_C);
	ZwGPIO_PinToOutput(PIN_RS485_CTRL);

	// Reset to default state
	ZwGPIO_WritePin(PIN_STPM_EN, FALSE);
	ZwGPIO_WritePin(PIN_STPM_DIR, FALSE);
	ZwGPIO_WritePin(PIN_STPM_STEP, FALSE);
	ZwGPIO_WritePin(PIN_OUT4, FALSE);
	ZwGPIO_WritePin(PIN_FAN, FALSE);
	ZwGPIO_WritePin(PIN_LED, FALSE);
	ZwGPIO_WritePin(PIN_OUT3, FALSE);
	ZwGPIO_WritePin(PIN_RS485_CTRL, FALSE);
	ZwGPIO_WritePin(PIN_SPIMUX_A, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_B, TRUE);
	ZwGPIO_WritePin(PIN_SPIMUX_C, TRUE);
	
	// Input pins
	ZwGPIO_PinToInput(PIN_SEN1, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_SEN2, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_HOME, TRUE, PQ_Sample6);
	ZwGPIO_PinToInput(PIN_SAFETY, TRUE, PQ_Sample6);
}
// ----------------------------------------

void DS18B20_Init()
{
	// Output pins
	ZwGPIO_PinToOutput(PIN_ADAPTER_ID_PWR);
	ZwGPIO_PinToOutput(PIN_ADAPTER_ID_CTRL);

	// Reset to default state
	ZwGPIO_WritePin(PIN_ADAPTER_ID_CTRL, FALSE);
	ZwGPIO_WritePin(PIN_ADAPTER_ID_PWR, TRUE);

	// Input pins
	ZwGPIO_PinToInput(PIN_ADAPTER_ID_DATA, FALSE, PQ_Async);
}

// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(ZbGPIO_FilterSafetyCircuit, "ramfuncs");
#endif
Boolean ZbGPIO_FilterSafetyCircuit(Boolean NewState)
{
	if(!NewState)
		SafetyCircuitCounter = 0;
	else
		SafetyCircuitCounter++;
	
	if(SafetyCircuitCounter > SC_FILTER_T)
		SafetyCircuitCounter = SC_FILTER_T;
	
	return (SafetyCircuitCounter >= SC_FILTER_T);
}
// ----------------------------------------

void ZbGPIO_CSMux(Int16U SPIDevice)
{
	ZwGPIO_WritePin(PIN_SPIMUX_A, SPIDevice & BIT0);
	ZwGPIO_WritePin(PIN_SPIMUX_B, SPIDevice & BIT1);
	ZwGPIO_WritePin(PIN_SPIMUX_C, SPIDevice & BIT2);
	DELAY_US(1);
}
// ----------------------------------------

// No more.
