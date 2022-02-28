// -----------------------------------------
// Controller logic
// ----------------------------------------

// Header
#include "SCCISlave.h"
#include "Controller.h"
//

// Includes
#include "SysConfig.h"
#include "Global.h"
//
#include "ZbBoard.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "DeviceProfile.h"
#include "TRM101.h"
#include "StepperMotor.h"
//

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
static volatile Boolean EnableAutoRelease = FALSE, CycleActive = FALSE, HeatingActive = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;
static volatile Int16U AutoReleaseTimeoutTicks, SlidingCounter;
//
volatile Int64U CONTROL_TimeCounter = 0, Timeout;
volatile Int32U FanTimeout = 0;
volatile DeviceState CONTROL_State = DS_None;
//
#pragma DATA_SECTION(CONTROL_Values_1, "data_mem");
Int16U CONTROL_Values_1[VALUES_x_SIZE];
//
#pragma DATA_SECTION(CONTROL_Values_1_32, "data_mem");
Int32U CONTROL_Values_1_32[VALUES_x_SIZE];
//
volatile Int16U CONTROL_Values_Counter = 0;
//
// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
//
static void CONTROL_HandleFanControl();
static void CONTROL_HandleClampActions();
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_SwitchToFault(Int16U Reason);
Boolean CONTROL_SlidingSensorOK();
Boolean CONTROL_PressureOK();

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes_16[EP_COUNT_16] = {0};
	Int16U EPSized_16[EP_COUNT_16] = {VALUES_x_SIZE};
	pInt16U EPCounters_16[EP_COUNT_16] = {(pInt16U)&CONTROL_Values_Counter};
	pInt16U EPDatas_16[EP_COUNT_16] = {CONTROL_Values_1};
	
	// Variables for endpoint configuration
	Int16U EPIndexes_32[EP_COUNT_32] = {0};
	Int16U EPSized_32[EP_COUNT_32] = {VALUES_x_SIZE};
	pInt16U EPCounters_32[EP_COUNT_32] = {(pInt16U)&CONTROL_Values_Counter};
	pInt16U EPDatas_32[EP_COUNT_32] = {(pInt16U)CONTROL_Values_1_32};
	
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {&ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM};
	
	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	DT_SaveFirmwareInfo(DEVICE_CAN_ADDRESS, 0);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();
	
	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService16(EPIndexes_16, EPSized_16, EPCounters_16, EPDatas_16);
	DEVPROFILE_InitEPService32(EPIndexes_32, EPSized_32, EPCounters_32, EPDatas_32);
	// Reset control values
	DEVPROFILE_ResetControlSection();
	
	// Sliding system init
	if(!BadClockDetected)
	{
		if(ZwSystem_GetDogAlarmFlag())
		{
			DataTable[REG_WARNING] = WARNING_WATCHDOG_RESET;
			ZwSystem_ClearDogAlarmFlag();
		}
		
		// Heating system init
		if(DataTable[REG_USE_HEATING])
		{
			TRMError dummy_error;
			
			// Terminate heating for sure
			TRM_Stop(TRM_CH1_ADDR, &dummy_error);
		}
	}
	else
	{
		CycleActive = TRUE;
		DataTable[REG_DISABLE_REASON] = DISABLE_BAD_CLOCK;
		CONTROL_SetDeviceState(DS_Disabled);
	}
}
// ----------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	
	// Update CAN bus status
	DEVPROFILE_UpdateCANDiagStatus();
	
	// Read sliding system state
	DataTable[REG_SLIDING_SENSOR] = CONTROL_SlidingSensorOK();
	
	// Process deferred procedures
	if(DPCDelegate)
	{
		FUNC_AsyncDelegate del = DPCDelegate;
		DPCDelegate = NULL;
		del();
	}
}
// ----------------------------------------

Boolean CONTROL_SlidingSensorOK()
{
	return (DataTable[REG_USE_SLIDING_SENSOR]) ? ZbGPIO_GetPowerConnectionState() : TRUE;
}
// ----------------------------------------

Int16U CONTROL_DevicePosition(Int16U DevTypeId)
{
	Int16U DevPos = 0;
	switch(DevTypeId)
	{
		case SC_Type_A2:
			DevPos = DataTable[REG_CASE_A2_DEF];
			break;
		case SC_Type_B0:
			DevPos = DataTable[REG_CASE_B0_DEF];
			break;
		case SC_Type_C1:
			DevPos = DataTable[REG_CASE_C1_DEF];
			break;
		case SC_Type_D:
			DevPos = DataTable[REG_CASE_D_DEF];
			break;
		case SC_Type_E:
			DevPos = DataTable[REG_CASE_E_DEF];
			break;
		case SC_Type_F:
			DevPos = DataTable[REG_CASE_F_DEF];
			break;
		default:
			DevPos = 0;
			break;
	}
	return DevPos;
}

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_UpdateLow, "ramfuncs");
#endif
void CONTROL_UpdateLow()
{
	CONTROL_HandleFanControl();
	CONTROL_HandleClampActions();
	ZbSU_UpdateTimeCounter(CONTROL_TimeCounter);
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_NotifyCANaFault, "ramfuncs");
#endif
void CONTROL_NotifyCANaFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANaFault(Flag);
}
// ----------------------------------------

static void CONTROL_FillWPPartDefault()
{
	// Set volatile states
	DataTable[REG_DEV_STATE] = (Int16U)DS_None;
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_DISABLE_REASON] = DISABLE_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
}
// ----------------------------------------

static void CONTROL_SetDeviceState(DeviceState NewState)
{
	if(NewState == DS_Clamping || NewState == DS_Moving)
		FanTimeout = FAN_TIMEOUT_TCK;
	
	// Delay before changing device state
	DELAY_US(1000L * DELAY_CHANGE_DEV_STATE);
	
	// Set new state
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
// ----------------------------------------

static void CONTROL_HandleFanControl()
{
	ZbGPIO_SwitchFan((FanTimeout > 0 || HeatingActive) ? TRUE : FALSE);
	if(FanTimeout > 0)
		--FanTimeout;
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	// Handle operation modes
	switch(CONTROL_State)
	{
		case DS_Homing:
			if(SM_IsHomingDone())
			{
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;
			
		case DS_Moving:
			if(ZbGPIO_IsSafetyOk())
			{
				if(SM_IsSlidingDone())
				{
					CONTROL_SetDeviceState(DS_Ready);
				}
			}
			else
			{
				SM_SetStopSteps();
				CONTROL_SwitchToFault(FAULT_SAFETY);
			}
			break;
			
		case DS_Clamping:
			if(SM_IsSlidingDone())
			{
				ZbGPIO_SwitchControlConnection(TRUE);
				CONTROL_SetDeviceState(DS_ClampingDone);
			}
			break;
			
		case DS_Fault:
		case DS_None:
			ZbGPIO_SwitchFan(FALSE);
			break;
			
		default:
			ZbGPIO_SwitchFan(TRUE);
			break;
			
	}
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_HOMING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				ZbGPIO_SwitchControlConnection(FALSE);
				SM_Homing();
				CONTROL_SetDeviceState(DS_Homing);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_GOTO_POSITION:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				ZbGPIO_SwitchControlConnection(FALSE);
				if(SM_GoToPositionFromReg(DataTable[REG_CUSTOM_POS], DataTable[REG_MAX_SPEED], 0, 0))
				{
					CONTROL_SetDeviceState(DS_Moving);
				}
				else
					*UserError = ERR_PARAMETER_OUT_OF_RNG;
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if(!ZbGPIO_GetPowerConnectionState())
				{
					DataTable[REG_PROBLEM] = PROBLEM_NONE;
					ZbGPIO_SwitchControlConnection(FALSE);
					Int16U LowSpeedPos = CONTROL_DevicePosition(DataTable[REG_DEV_CASE]);
					if(SM_GoToPositionFromReg(DataTable[REG_CUSTOM_POS], DataTable[REG_MAX_SPEED], LowSpeedPos,
							SM_MIN_SPEED))
					{
						CONTROL_SetDeviceState(DS_Moving);
					}
					else
						*UserError = ERR_PARAMETER_OUT_OF_RNG;
				}
				else
				{
					CONTROL_SwitchToFault(FAULT_POWER_CON);
					*UserError = ERR_SLIDING_SYSTEM;
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				ZbGPIO_SwitchControlConnection(FALSE);
				if(SM_GoToPositionFromReg(0, DataTable[REG_MAX_SPEED], 0, 0))
				{
					CONTROL_SetDeviceState(DS_Moving);
				}
				else
					*UserError = ERR_PARAMETER_OUT_OF_RNG;
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HALT:
			{
				SM_SetStopSteps();
				CONTROL_SetDeviceState(DS_Halt);
			}
			break;
			
		case ACT_RELEASE_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
			{
				ZbGPIO_SwitchPowerConnection(FALSE);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
			{
				ZbGPIO_SwitchPowerConnection(TRUE);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_SET_TEMPERATURE:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					if(DataTable[REG_TEMP_SETPOINT] < TRM_TEMP_THR)
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None)
							TRM_Stop(TRM_CH1_ADDR, &error);
						
						HeatingActive = FALSE;
					}
					else
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None)
							TRM_Start(TRM_CH1_ADDR, &error);
						
						HeatingActive = TRUE;
					}
					
					if(error != TRME_None)
					{
						CONTROL_SwitchToFault(FAULT_TRM);
						
						DataTable[REG_TRM_ERROR] = error;
						*UserError = ERR_TRM_COMM_ERR;
					}
				}
				else
					DataTable[REG_TEMP_CH1] = DataTable[REG_TEMP_SETPOINT];
			}
			break;
			
		case ACT_CLR_FAULT:
			{
				if(CONTROL_State == DS_Fault)
					CONTROL_SetDeviceState(DS_None);
				else if(CONTROL_State == DS_Disabled)
					*UserError = ERR_OPERATION_BLOCKED;
				
				DataTable[REG_PROBLEM] = PROBLEM_NONE;
				DataTable[REG_WARNING] = WARNING_NONE;
				DataTable[REG_FAULT_REASON] = FAULT_NONE;
			}
			break;
			
		case ACT_CLR_WARNING:
			DataTable[REG_PROBLEM] = PROBLEM_NONE;
			DataTable[REG_WARNING] = WARNING_NONE;
			break;
			
		case ACT_CLR_HALT:
			if(CONTROL_State == DS_Halt)
				CONTROL_SetDeviceState(DS_Ready);
			break;
			
		case ACT_DBG_READ_TRM_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_READ_TRM_POWER:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadPower(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_TRM_START:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Start(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_TRM_STOP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Stop(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
					
					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DBG_CONNECT_CONTROL:
			ZbGPIO_SwitchControlConnection(TRUE);
			break;
			
		case ACT_DBG_DISCONNECT_CONTROL:
			ZbGPIO_SwitchControlConnection(FALSE);
			break;
			
		default:
			return FALSE;
	}
	
	return TRUE;
}
// ----------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = Reason;
}
// ----------------------------------------
