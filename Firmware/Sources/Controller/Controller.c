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

volatile Int64U FanTimeout = 0, CONTROL_TimeCounter = 0, Timeout;
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = DSS_None;

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
static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_PreparePositioningX(Int16U NewPosition, Int16U SlowDownDistance,
		Int16U MaxSpeed, Int16U LowSpeed, Int16U MinSpeed);
void CONTROL_PreparePositioning();
void CONTROL_PrepareHomingOffset();
void CONTROL_PrepareClamping(Boolean Clamp);
void CONTROL_Halt();

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
	
	SM_ResetZeroPoint();
	ZwTimer_StartT1();

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
		CONTROL_SetDeviceState(DS_Disabled, DSS_None);
	}
}
// ----------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	DEVPROFILE_UpdateCANDiagStatus();
	
	DataTable[REG_SAFETY_SENSOR] = ZbGPIO_IsSafetySensorOk();
	DataTable[REG_HOMING_SENSOR] = ZbGPIO_HomeSensorActuate();
	DataTable[REG_BUS_TOOLING_SENSOR] = !ZbGPIO_IsBusToolingSensorOk();
	DataTable[REG_ADAPTER_TOOLING_SENSOR] = ZbGPIO_IsAdapterToolingSensorOk();
	
	// Process deferred procedures
	if(DPCDelegate)
	{
		FUNC_AsyncDelegate del = DPCDelegate;
		DPCDelegate = NULL;
		del();
	}
}
// ----------------------------------------

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

static void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	if(NewState == DS_Clamping || NewState == DS_Position)
		FanTimeout = CONTROL_TimeCounter + FAN_TIMEOUT;
	
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;

	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_SUBSTATE] = NewSubState;
}
// ----------------------------------------

static void CONTROL_HandleFanControl()
{
	ZbGPIO_SwitchFan((FanTimeout > CONTROL_TimeCounter) || HeatingActive);
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	static Int64U Timeout = 0;

	switch(CONTROL_State)
	{
		case DS_Homing:
		case DS_Position:
		case DS_Clamping:
		case DS_ClampingRelease:
			if(DataTable[REG_USE_SAFETY_SENSOR] && !ZbGPIO_IsSafetySensorOk())
				CONTROL_Halt();
			break;
	}

	// Обработка общей логики отключения управления
	switch(CONTROL_SubState)
	{
		case DSS_Com_CheckControl:
			if(ZbGPIO_IsControlConnected())
			{
				Timeout = CONTROL_TimeCounter + PNEUMATIC_PAUSE;
				ZbGPIO_SwitchControlConnection(FALSE);
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ControlRelease);
			}
			else
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
			break;

		case DSS_Com_ControlRelease:
			if(CONTROL_TimeCounter > Timeout)
				CONTROL_SetDeviceState(CONTROL_State, DSS_Com_ReleaseDone);
			break;
	}

	// Обработка машины подсостояний
	switch(CONTROL_State)
	{
		case DS_Homing:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					SM_Homing(DataTable[REG_HOMING_SPEED]);
					CONTROL_SetDeviceState(CONTROL_State, DSS_HomingSearchSensor);
					break;

				case DSS_HomingSearchSensor:
					if(SM_IsHomingDone())
					{
						Timeout = CONTROL_TimeCounter + HOMING_PAUSE;
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingPause);
					}
					break;

				case DSS_HomingPause:
					if(CONTROL_TimeCounter > Timeout)
					{
						CONTROL_PrepareHomingOffset();
						CONTROL_SetDeviceState(CONTROL_State, DSS_HomingMakeOffset);
					}
					break;

				case DSS_HomingMakeOffset:
					if(SM_IsPositioningDone())
					{
						SM_ResetZeroPoint();
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					}
			}
			break;

		case DS_Position:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					CONTROL_PreparePositioning();
					CONTROL_SetDeviceState(CONTROL_State, DSS_PositionOperating);
					break;

				case DSS_PositionOperating:
					if(SM_IsPositioningDone())
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					break;
			}
			break;

		case DS_Clamping:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					{
						Boolean IsBusOk = ZbGPIO_IsBusToolingSensorOk();
						Boolean IsAdapterOk = !ZbGPIO_IsAdapterToolingSensorOk();

						if(!DataTable[REG_USE_TOOLING_SENSOR] || (IsBusOk && IsAdapterOk))
						{
							CONTROL_PrepareClamping(TRUE);
							CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingOperating);
						}
						else
							CONTROL_SwitchToFault(IsBusOk ? FAULT_ADAPTER_SEN : FAULT_BUS_SEN);
					}
					break;

				case DSS_ClampingOperating:
					if(SM_IsPositioningDone())
					{
						if(DataTable[REG_DEV_CASE] == SC_Type_C1 || DataTable[REG_DEV_CASE] == SC_Type_F)
							CONTROL_SetDeviceState(DS_ClampingDone, DSS_None);
						else
						{
							ZbGPIO_SwitchControlConnection(TRUE);
							Timeout = CONTROL_TimeCounter + PNEUMATIC_PAUSE;
							CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingConnectControl);
						}
					}
					break;

				case DSS_ClampingConnectControl:
					if(CONTROL_TimeCounter > Timeout)
						CONTROL_SetDeviceState(DS_ClampingDone, DSS_None);
					break;
			}
			break;

		case DS_ClampingRelease:
			switch(CONTROL_SubState)
			{
				case DSS_Com_ReleaseDone:
					CONTROL_PrepareClamping(FALSE);
					CONTROL_SetDeviceState(CONTROL_State, DSS_ClampingReleaseOperating);
					break;

				case DSS_ClampingReleaseOperating:
					if(SM_IsPositioningDone())
						CONTROL_SetDeviceState(DS_Ready, DSS_None);
					break;
			}
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
				CONTROL_SetDeviceState(DS_Homing, DSS_Com_CheckControl);
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_GOTO_POSITION:
			if(CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_Position, DSS_Com_CheckControl);
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_Ready)
			{
				ZbGPIO_SwitchPowerConnection(TRUE);
				CONTROL_SetDeviceState(DS_Clamping, DSS_Com_CheckControl);
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;
			
		case ACT_RELEASE_CLAMPING:
			if(CONTROL_State == DS_Halt || CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_ClampingRelease, DSS_Com_CheckControl);
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;
			
		case ACT_HALT:
			CONTROL_Halt();
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
					CONTROL_SetDeviceState(DS_None, DSS_None);
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
				CONTROL_SetDeviceState(DS_Ready, DSS_None);
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

void CONTROL_Halt()
{
	SM_RequestStop();
	CONTROL_SetDeviceState(DS_Halt, DSS_None);
}
// ----------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, DSS_None);
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

void CONTROL_PrepareClamping(Boolean Clamp)
{
	if(Clamp)
		CONTROL_PreparePositioningX(DataTable[DataTable[REG_DEV_CASE]], DataTable[REG_SLOW_DOWN_DIST],
				DataTable[REG_CLAMP_SPEED_MAX], DataTable[REG_CLAMP_SPEED_LOW], DataTable[REG_CLAMP_SPEED_MIN]);
	else
		CONTROL_PreparePositioningX(0, 0,
				DataTable[REG_CLAMP_SPEED_MAX], DataTable[REG_CLAMP_SPEED_LOW], DataTable[REG_CLAMP_SPEED_MIN]);
}
// ----------------------------------------

void CONTROL_PreparePositioning()
{
	CONTROL_PreparePositioningX(DataTable[REG_CUSTOM_POS], DataTable[REG_SLOW_DOWN_DIST],
			DataTable[REG_POS_SPEED_MAX], DataTable[REG_POS_SPEED_LOW], DataTable[REG_POS_SPEED_MIN]);
}
// ----------------------------------------

void CONTROL_PrepareHomingOffset()
{
	CONTROL_PreparePositioningX(DataTable[REG_HOMING_OFFSET], 0,
			DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED], DataTable[REG_HOMING_SPEED]);
}
// ----------------------------------------
