﻿// Header
#include "Controller.h"
//
// Includes
#include "Board.h"
#include "Delay.h"
#include "DeviceProfile.h"
#include "Interrupts.h"
#include "LowLevel.h"
#include "SysConfig.h"
#include "DebugActions.h"
#include "Diagnostic.h"
#include "Logic.h"
#include "BCCIxParams.h"
#include "Constraints.h"
#include "Measurement.h"
#include "math.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
static Boolean CycleActive = false;
static float ActualPressureValue = 0;
//
volatile Int64U CONTROL_TimeCounter = 0;

// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
Int16U CONTROL_CSMPrepareLogic();
void CONTROL_ClampLogic();
void CONTROL_SamplePressureValue();

// Functions
//
void CONTROL_Init()
{
	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, 0);
	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	
	// Сброс значений
	DEVPROFILE_ResetControlSection();
	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;

	CONTROL_SetDeviceState(DS_None, SS_None);
}
//------------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	
	LOGIC_UpdateSensors();
	CONTROL_SamplePressureValue();
	CONTROL_ClampLogic();
	
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
				CONTROL_SetDeviceState(DS_None, SS_None);
			else if(CONTROL_State != DS_Ready)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_DISABLE_POWER:
			if(CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_None, SS_None);
			else if(CONTROL_State != DS_None)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
			{
				CONTROL_SetDeviceState(DS_None, SS_None);
				DataTable[REG_FAULT_REASON] = DF_NONE;
			}
			break;
			
		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;
			
		case ACT_CLAMP:
			if(CONTROL_State == DS_Ready)
			{
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				DataTable[REG_OP_RESULT] = OPRESULT_NONE;

				Int16U PrepareResult = CONTROL_CSMPrepareLogic();
				if(PrepareResult == PROBLEM_NONE)
					CONTROL_SetDeviceState(DS_Clamping, SS_BlockAdapters);
				else
				{
					DataTable[REG_PROBLEM] = PrepareResult;
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				}
			}
			else if(CONTROL_State == DS_Clamping)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_RELEASE:
			if(CONTROL_State == DS_ClampingDone)
			{
				DataTable[REG_OP_RESULT] = OPRESULT_NONE;
				CONTROL_SetDeviceState(DS_ClampingRelease, SS_StartRelease);
			}
			else if(CONTROL_State == DS_ClampingRelease)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;
			
		default:
			return DIAG_HandleDiagnosticAction(ActionID, pUserError);
			
	}
	return true;
}
//-----------------------------------------------

Int16U CONTROL_CSMPrepareLogic()
{
	DUTType TopID = LOGIC_AdapterIDMatch(LL_MeasureIDTop());
	DUTType BotID = LOGIC_AdapterIDMatch(LL_MeasureIDBot());

	if(TopID != DataTable[REG_ID_ADPTR_SET])
		return PROBLEM_TOP_ADAPTER_MISMATCHED;

	else if(BotID != DataTable[REG_ID_ADPTR_SET])
		return PROBLEM_BOT_ADAPTER_MISMATCHED;

	else if(!LL_GetStateLimitSwitchTopAdapter())
		return PROBLEM_TOP_ADAPTER_OPENED;

	else if(!LL_GetStateLimitSwitchBotAdapter())
		return PROBLEM_BOT_ADAPTER_OPENED;

	else
		return PROBLEM_NONE;
}
//-----------------------------------------------

void CONTROL_ClampLogic()
{
	float MeasuredError = ((float)DataTable[REG_SET_PRESSURE_VALUE] - ActualPressureValue)
			/ DataTable[REG_SET_PRESSURE_VALUE] * 100;
	float AllowedError = (float)DataTable[REG_ALLOWED_ERROR] / 10;
	
	static Int64U Delay = 0;

	if(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingRelease)
	{
		switch(CONTROL_SubState)
		{
			case SS_BlockAdapters:
				Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
				LL_SetStatePneumTOP(true);
				LL_SetStatePneumBOT(true);
				CONTROL_SetDeviceState(DS_Clamping, SS_BlockDelay);
				break;

			case SS_BlockDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
					LL_SetStateIndADPTR(true);
					LL_SetStatePneumDUT(true);
					CONTROL_SetDeviceState(DS_Clamping, SS_ClampDelay);
				}
				break;

			case SS_ClampDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					LL_SetStateIndCSM(true);
					LL_SetStateSFOutput(true);
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
					CONTROL_SetDeviceState(DS_ClampingDone, SS_None);
				}
				break;

			case SS_StartRelease:
				Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
				LL_SetStateSFOutput(false);
				LL_SetStateIndCSM(false);
				LL_SetStatePneumDUT(false);
				CONTROL_SetDeviceState(DS_ClampingRelease, SS_ReleaseDelay);
				break;

			case SS_ReleaseDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					Delay = CONTROL_TimeCounter + PNEUMO_DELAY;
					LL_SetStatePneumTOP(false);
					LL_SetStatePneumBOT(false);
					CONTROL_SetDeviceState(DS_ClampingRelease, SS_UnblockDelay);
				}
				break;
				
			case SS_UnblockDelay:
				if(CONTROL_TimeCounter > Delay)
				{
					LL_SetStateIndADPTR(false);
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
				break;
				
			default:
				break;
		}
	}
}
//-----------------------------------------------

void CONTROL_SamplePressureValue()
{
	ActualPressureValue = MEASURE_GetPressureValue();
	DataTable[REG_PRESSURE_VALUE] = ActualPressureValue;
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, SS_None);
	DataTable[REG_FAULT_REASON] = Reason;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
	
	CONTROL_SubState = NewSubState;
	DataTable[REG_SUB_STATE] = NewSubState;
}
//------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------

