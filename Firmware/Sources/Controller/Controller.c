// ----------------------------------------
// Controller logic
// ----------------------------------------

// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"
#include "Global.h"
#include "SCCISlave.h"
#include "ZbBoard.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "DeviceProfile.h"
#include "TRM101.h"
#include "StepperMotor.h"
#include "StepperMotorDiag.h"

// Types
typedef void (*FUNC_AsyncDelegate)();

// Variables
static volatile Boolean CycleActive = FALSE, HeatingActive = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;

volatile Int64U CONTROL_TimeCounter = 0, Timeout;
volatile Int32U FanTimeout = 0;
volatile DeviceState CONTROL_State = DS_None;

#pragma DATA_SECTION(CONTROL_Values_1, "data_mem");
Int16U CONTROL_Values_1[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_1_32, "data_mem");
Int32U CONTROL_Values_1_32[VALUES_x_SIZE];
volatile Int16U CONTROL_Values_Counter = 0;

// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
static void CONTROL_HandleFanControl();
static void CONTROL_HandleClampActions();
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_SwitchToFault(Int16U Reason);
Boolean CONTROL_IsToolingSensorOK();
void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U LowSpeed, Int16U MinSpeed);
void CONTROL_PreparePositioning();
void CONTROL_PrepareHomingOffset();

// Functions
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
	DEVPROFILE_UpdateCANDiagStatus();
	
	DataTable[REG_SAFETY_SENSOR] = ZbGPIO_IsSafetySensorOk();
	DataTable[REG_TOOLING_SENSOR] = ZbGPIO_IsToolingSensorOk();
	DataTable[REG_HOMING_SENSOR] = ZbGPIO_HomeSensorActuate();
	
	// Process deferred procedures
	if(DPCDelegate)
	{
		FUNC_AsyncDelegate del = DPCDelegate;
		DPCDelegate = NULL;
		del();
	}
}
// ----------------------------------------

Boolean CONTROL_IsToolingSensorOK()
{
	return (DataTable[REG_USE_TOOLING_SENSOR]) ? ZbGPIO_IsToolingSensorOk() : TRUE;
}
// ----------------------------------------

Int16U CONTROL_DevicePosition(Int16U DevTypeId)
{
	Int16U DevPos = 0;
	switch(DevTypeId)
	{
		case SC_Type_A2:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_A2];
			break;
		case SC_Type_B0:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_B0];
			break;
		case SC_Type_C1:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_C1];
			break;
		case SC_Type_D:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_D];
			break;
		case SC_Type_E:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_E];
			break;
		case SC_Type_F:
			DevPos = DataTable[REG_CLAMP_HEIGHT_CASE_F];
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
				CONTROL_PrepareHomingOffset();
				CONTROL_SetDeviceState(DS_HomingOffset);
			}
			break;
			
		case DS_HomingOffset:
			if(SM_IsPositioningDone())
			{
				SM_ResetZeroPoint();
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;

		case DS_Moving:
			if(SM_IsPositioningDone())
				CONTROL_SetDeviceState(DS_Ready);
			break;
			
		case DS_Clamping:
			if(SM_IsPositioningDone())
			{
				ZbGPIO_SwitchControlConnection(TRUE);
				CONTROL_SetDeviceState(DS_ClampingDone);
			}
			break;
			
		default:
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
				ZbGPIO_SwitchControlConnection(FALSE);
				SM_Homing(DataTable[REG_HOMING_SPEED]);
				CONTROL_SetDeviceState(DS_Homing);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_GOTO_POSITION:
			if(CONTROL_State == DS_Ready)
			{
				ZbGPIO_SwitchControlConnection(FALSE);
				CONTROL_PreparePositioning(DataTable[REG_CUSTOM_POS]);
				CONTROL_SetDeviceState(DS_Moving);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_Ready)
			{
				if(CONTROL_IsToolingSensorOK())
				{
					ZbGPIO_SwitchControlConnection(FALSE);
					Int16U LowSpeedPos = CONTROL_DevicePosition(DataTable[REG_DEV_CASE]);
					//SM_GoToPositionFromReg(DataTable[REG_CUSTOM_POS], DataTable[REG_POS_MAX_SPEED], LowSpeedPos, SM_MIN_SPEED);
					CONTROL_SetDeviceState(DS_Moving);
				}
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				ZbGPIO_SwitchControlConnection(FALSE);
				//SM_GoToPositionFromReg(0, DataTable[REG_POS_MAX_SPEED], 0, 0);
				CONTROL_SetDeviceState(DS_Moving);
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HALT:
			CONTROL_SetDeviceState(DS_Halt);
			break;
			
		case ACT_RELEASE_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
				ZbGPIO_SwitchPowerConnection(FALSE);
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HOLD_ADAPTER:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Ready)
				ZbGPIO_SwitchPowerConnection(TRUE);
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
				
				DataTable[REG_FAULT_REASON] = FAULT_NONE;
			}
			break;
			
		case ACT_CLR_WARNING:
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
			
		case ACT_DBG_MOTOR_START:
			SMD_ConnectHandler();
			break;
			
		case ACT_DBG_MOTOR_STOP:
			SMD_RequstStop();
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

void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U LowSpeed, Int16U MinSpeed)
{
	SM_Config Config;
	Config.NewPosition = NewPosition;
	Config.SlowDownDistance = SlowDownDistance;
	Config.MaxSpeed = MaxSpeed;
	Config.LowSpeed = LowSpeed;
	Config.MinSpeed = MinSpeed;

	SM_GoToPosition(&Config);
}
// ----------------------------------------

void CONTROL_PreparePositioning()
{
	CONTROL_PreparePositioningX(DataTable[REG_CUSTOM_POS], DataTable[REG_SLOW_DOWN_DIST],
			DataTable[REG_POS_MAX_SPEED], DataTable[REG_POS_LOW_SPEED], DataTable[REG_POS_MIN_SPEED]);
}
// ----------------------------------------

void CONTROL_PrepareHomingOffset()
{
	CONTROL_PreparePositioningX(DataTable[REG_HOMING_OFFSET], 0,
			DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED]);
}
// ----------------------------------------
