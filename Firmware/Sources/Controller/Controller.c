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
#include "TemperatureFeedback.h"
#include "Clamp.h"
#include "ClampControl.h"
#include "PI130.h"
//
#include "AnalogOutput.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();


// Variables
//
static volatile Boolean EnableAutoRelease = FALSE, CycleActive = FALSE, MuteRegulator = FALSE, HeatingActive = FALSE;
static volatile FUNC_AsyncDelegate DPCDelegate = NULL;
static volatile Int16U AutoReleaseTimeoutTicks, SlidingCounter;
//
volatile Int64U CONTROL_TimeCounter = 0, CONTROL_HSCounter = 0, Timeout;
volatile Int32U FanTimeout = 0;
volatile DeviceState CONTROL_State = DS_None;
volatile MasterState CONTROL_MasterState = MS_None;
//
Int16U CONTROL_Values_Time[VALUES_x_SIZE];
Int32U CONTROL_Values_Position[VALUES_x_SIZE];
Int16U CONTROL_Values_Torque[VALUES_x_SIZE];
volatile Int16U CONTROL_Values_Counter = 0;
//
volatile Int16U CONTROL_RS485_Rx[VALUES_RX_SIZE];
volatile Int16U CONTROL_Rx_Counter = 0;
//
// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;


// Forward functions
//
static void CONTROL_HandleClampActions();
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_SetMasterState(MasterState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
static void CONTROL_ResetScopes();
Boolean CONTROL_SlidingSensorOK();
Boolean CONTROL_PressureOK();
void CONTROL_ProcessMasterEvents();


// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	Int16U EPIndexes_16[EP_COUNT_16] = {EP16_Data_Time, EP16_Data_Torque, EP16_RS485_Rx};
	Int16U EPSized_16[EP_COUNT_16] = {VALUES_x_SIZE, VALUES_x_SIZE, VALUES_RX_SIZE};
	pInt16U EPCounters_16[EP_COUNT_16] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Rx_Counter};
	pInt16U EPDatas_16[EP_COUNT_16] = {CONTROL_Values_Time, CONTROL_Values_Torque, (pInt16U)CONTROL_RS485_Rx};

	Int16U EPIndexes_32[EP_COUNT_32] = {EP32_Data_Position};
	Int16U EPSized_32[EP_COUNT_32] = {VALUES_x_SIZE};
	pInt16U EPCounters_32[EP_COUNT_32] = {(pInt16U)&CONTROL_Values_Counter};
	pInt16U EPDatas_32[EP_COUNT_32] = {(pInt16U)CONTROL_Values_Position};

	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = { &ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM };

	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();

	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService16(EPIndexes_16, EPSized_16, EPCounters_16, EPDatas_16);
	DEVPROFILE_InitEPService32(EPIndexes_32, EPSized_32, EPCounters_32, EPDatas_32);
	// Reset control values
	DEVPROFILE_ResetControlSection();

	// Init analog input parameters
	ZbAnanlogInput_Init();

	// Sliding system init
	ZbGPIO_PneumoPushOut(FALSE);
	ZbGPIO_PneumoPushUp(FALSE);

	if(!BadClockDetected)
	{
		if(ZwSystem_GetDogAlarmFlag())
		{
			DataTable[REG_WARNING] = WARNING_WATCHDOG_RESET;
			ZwSystem_ClearDogAlarmFlag();
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

	CONTROL_ProcessMasterEvents();

	// Process deferred procedures
	if(DPCDelegate)
	{
		FUNC_AsyncDelegate del = DPCDelegate;
		DPCDelegate = NULL;
		del();
	}
}
// ----------------------------------------

void inline CONTROL_RequestDPC(FUNC_AsyncDelegate Action)
{
	DPCDelegate = Action;
}
// ----------------------------------------

void inline CONTROL_RequestClampRelease()
{
	if (CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready || CONTROL_State == DS_Halt || CONTROL_State == DS_None)
	{
		// prevent interrupt by regulator
		MuteRegulator = TRUE;

		ZbGPIO_EnablePowerSwitch(FALSE);

		DataTable[REG_PROBLEM] = PROBLEM_NONE;
		CLAMPCTRL_StartClampingRelease((CONTROL_State == DS_ClampingDone) ? FALSE : TRUE);
		CONTROL_SetDeviceState(DS_ClampingRelease);

		// resume regulator
		MuteRegulator = FALSE;
	}
}
// ----------------------------------------

Boolean CONTROL_SlidingSensorOK()
{
	return (DataTable[REG_USE_SLIDING_SENSOR]) ? ZbGPIO_GetS3State() : TRUE;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(CONTROL_UpdateLow, "ramfuncs");
#endif
void CONTROL_UpdateLow()
{
	CONTROL_HandleClampActions();
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

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(CONTROL_NotifyCANbFault, "ramfuncs");
#endif
void CONTROL_NotifyCANbFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANbFault(Flag);
}
// ----------------------------------------

void CONTROL_NotifyCANopenFault(CANopenErrCode ErrorCode, Int16U Index, Int16U SubIndex, Int32U Value)
{
	DataTable[REG_FAULT_REASON] = FAULT_CANOPEN;
	CONTROL_SetDeviceState(DS_Fault);

	DataTable[REG_CANO_ERR_CODE] = ErrorCode;
	DataTable[REG_CANO_ERR_INDEX] = Index;
	DataTable[REG_CANO_ERR_SUBINDEX] = SubIndex;
	DataTable[REG_CANO_ERR_DATA] = Value;
	DataTable[REG_CANO_ERR_DATA_32] = Value >> 16;
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

	DataTable[REG_CANO_ERR_CODE] = 0;
	DataTable[REG_CANO_ERR_INDEX] = 0;
	DataTable[REG_CANO_ERR_SUBINDEX] = 0;
	DataTable[REG_CANO_ERR_DATA] = 0;
	DataTable[REG_CANO_ERR_DATA_32] = 0;
}
// ----------------------------------------

static void CONTROL_SetDeviceState(DeviceState NewState)
{
	if (NewState == DS_Clamping || NewState == DS_ClampingUpdate)
		FanTimeout = FAN_TIMEOUT_TCK;

	// Handle power switch
	ZbGPIO_EnablePowerSwitch(NewState == DS_ClampingDone ? TRUE : FALSE);

	// Delay before changing device state
	DELAY_US(1000L * DELAY_CHANGE_DEV_STATE);

	// Set new state
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
// ----------------------------------------

Boolean CONTROL_CheckLenzeError()
{
	Int32U err = CLAMP_ReadError();

	if (err)
	{
		DataTable[REG_DRV_ERROR] = err;
		DataTable[REG_FAULT_REASON] = DISABLE_LENZE_ERROR;
		CONTROL_SetDeviceState(DS_Disabled);
	}

	return err;
}
// ----------------------------------------

Boolean CONTROL_PressureOK()
{
	return (DataTable[REG_USE_AIR_CONTROL]) ? ZbGPIO_PressureOK() : TRUE;
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	static Int64U ClampingDoneCounterCopy = 0;

	// Extended data logging
	CLAMPCTRL_XLog(CONTROL_State);

	// Disable any actions with motor driver
	if (MuteRegulator) return;

	// Handle quick stop request
	switch(CONTROL_State)
	{
		case DS_Homing:
		case DS_Position:
		case DS_Clamping:
		case DS_ClampingDone:
		case DS_ClampingUpdate:
		case DS_ClampingRelease:
			{
				if (CLAMP_IsQSPActive())
				{
					DINT;
					CLAMP_CompleteOperation(TRUE);
					CONTROL_SetDeviceState(DS_Halt);
					EINT;
				}
			}
			break;

		default:
			break;
	}

	// Handle operation modes
	switch(CONTROL_State)
	{
		case DS_Homing:
			if (CLAMP_IsHomingDone())
			{
				CLAMP_CompleteOperation(FALSE);

				// Go to initial position
				CLAMP_SpeedTorqueLimits(DataTable[REG_POSITION_SPEED_LIMIT], DataTable[REG_POSITION_TORQUE_LIMIT]);
				CLAMP_GoToPosition_mm(FALSE, DataTable[REG_CLAMPING_RLS_POS]);
				CONTROL_SetDeviceState(DS_Position);
			}
			break;

		case DS_Position:
			if (CLAMP_IsTargetReached())
			{
				CLAMP_CompleteOperation(FALSE);
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;

		case DS_Clamping:
			if (CLAMPCTRL_IsClampingDone())
			{
				// clamping process
				ClampingDoneCounterCopy = CONTROL_TimeCounter;
				CONTROL_SetDeviceState(DS_ClampingDone);
			}
			break;

		case DS_ClampingDone:
			{
				// post-clamping regulation
				CLAMPCTRL_IsClampingDone();
				// check release condition
				if ((EnableAutoRelease && (CONTROL_TimeCounter - ClampingDoneCounterCopy > AutoReleaseTimeoutTicks)) || !CONTROL_PressureOK())
					CONTROL_RequestDPC(&CONTROL_RequestClampRelease);
			}
			break;

		case DS_ClampingUpdate:
			{
				CLAMPCTRL_ClampingUpdateRequest();
				CONTROL_SetDeviceState(DS_Clamping);
			}
			break;

		case DS_ClampingRelease:
			if (CLAMPCTRL_IsClampingReleaseDone())
			{
				CLAMP_CompleteOperation(TRUE);
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;

		case DS_Sliding:
			{
				if (CONTROL_TimeCounter < Timeout)
				{
					if (CONTROL_SlidingSensorOK())
						++SlidingCounter;
					else
						SlidingCounter = 0;

					if (SlidingCounter > SLS_BOUNCE_COUNTER)
					{
						ZbGPIO_PneumoPushUp(FALSE);
						CONTROL_SetDeviceState(DS_Ready);
					}
				}
				else
				{
					DataTable[REG_FAULT_REASON] = FAULT_SLIDING;
					CONTROL_SetDeviceState(DS_Fault);
				}
			}
			break;
	}
}
// ----------------------------------------

static void CONTROL_ResetScopes()
{
	DEVPROFILE_ResetEPReadState();
	DEVPROFILE_ResetScopes16(0);
	DEVPROFILE_ResetScopes32(0);
	CONTROL_Values_Counter = 0;
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_HOMING:
			if (CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if (CONTROL_CheckLenzeError())
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					DataTable[REG_PROBLEM] = PROBLEM_NONE;
					CLAMP_HomingStart();
					CONTROL_SetDeviceState(DS_Homing);
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_GOTO_POSITION:
			if (CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if (CONTROL_CheckLenzeError() ||
					((CONTROL_State == DS_None || CONTROL_State == DS_Halt) && !CLAMP_IsHomingPosAvailable()))
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					CONTROL_ResetScopes();
					Int32U Position = ((Int32U)DataTable[REG_CUSTOM_POS_32] << 16) | DataTable[REG_CUSTOM_POS];

					DataTable[REG_PROBLEM] = PROBLEM_NONE;
					CLAMP_SpeedTorqueLimits(DataTable[REG_POSITION_SPEED_LIMIT], DataTable[REG_POSITION_TORQUE_LIMIT]);
					CLAMP_GoToPosition_mm(TRUE, (Int32S)Position);
					CONTROL_SetDeviceState(DS_Position);

					if(DataTable[REG_DBG_CAN_MONITOR_DELAY])
						DELAY_US(DataTable[REG_DBG_CAN_MONITOR_DELAY] * 1000L);
					ZwTimer_StartT1();
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DBGCAN_LOG_SINGLE:
			CONTROL_PDOMonitor();
			break;

		case ACT_START_CLAMPING:
			if (CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if (!CONTROL_PressureOK())
				{
					*UserError = ERR_NO_AIR_PRESSURE;
				}
				else if (!CONTROL_SlidingSensorOK())
				{
					*UserError = ERR_SLIDING_SYSTEM;
				}
				else if (CONTROL_CheckLenzeError() ||
					((CONTROL_State == DS_None || CONTROL_State == DS_Halt) && !CLAMP_IsHomingPosAvailable()))
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					AutoReleaseTimeoutTicks = ((Int32U)DataTable[REG_MAX_CONT_FORCE_TIMEOUT] * CS_MONITORING_FREQ) / 1000;
					EnableAutoRelease = (DataTable[REG_FORCE_VAL] >= DataTable[REG_MAX_CONT_FORCE] && DataTable[REG_USE_CLAMP_BREAK] == 0) ? TRUE : FALSE;

					DataTable[REG_PROBLEM] = PROBLEM_NONE;
					CONTROL_ResetScopes();
					CLAMPCTRL_StartClamping();
					CONTROL_SetDeviceState(DS_Clamping);
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_CLAMPING_UPDATE:
			if (CONTROL_State == DS_ClampingDone)
			{
				// prevent interrupt by regulator
				MuteRegulator = TRUE;

				if (CONTROL_CheckLenzeError())
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					AutoReleaseTimeoutTicks = ((Int32U)DataTable[REG_MAX_CONT_FORCE_TIMEOUT] * CS_MONITORING_FREQ) / 1000;
					EnableAutoRelease = (DataTable[REG_FORCE_VAL] >= DataTable[REG_MAX_CONT_FORCE] && DataTable[REG_USE_CLAMP_BREAK] == 0) ? TRUE : FALSE;

					DataTable[REG_PROBLEM] = PROBLEM_NONE;
					CONTROL_SetDeviceState(DS_ClampingUpdate);
				}

				// resume regulator
				MuteRegulator = FALSE;
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_RELEASE_CLAMPING:
			if (CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready || CONTROL_State == DS_Halt  || CONTROL_State == DS_None)
			{
				// prevent interrupt by regulator
				MuteRegulator = TRUE;

				if (CONTROL_CheckLenzeError())
					*UserError = ERR_DEVICE_NOT_READY;
				else
					CONTROL_RequestClampRelease();

				// resume regulator
				MuteRegulator = FALSE;
			}
			else if (CONTROL_State != DS_ClampingRelease)
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_HALT:
			CONTROL_SetDeviceState(DS_Halt);
		case ACT_INHIBIT:
			DINT;
			CLAMP_CompleteOperation(TRUE);
			EINT;
			break;

		case ACT_SLIDING_PUSH_OUT:
			{
				if (CONTROL_State == DS_Ready)
				{
					ZbGPIO_PneumoPushUp(TRUE);
					DELAY_US(SLS_PUSH_UP_TO_OUT_PAUSE * 1000L);
					ZbGPIO_PneumoPushOut(TRUE);
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_SLIDING_PUSH_IN:
			{
				if (CONTROL_State == DS_Ready)
				{
					ZbGPIO_PneumoPushUp(TRUE);
					DELAY_US(SLS_PUSH_UP_TO_OUT_PAUSE * 1000L);
					ZbGPIO_PneumoPushOut(FALSE);

					SlidingCounter = 0;
					Timeout = CONTROL_TimeCounter + SLS_PUSH_IN_TIMEOUT;
					CONTROL_SetDeviceState(DS_Sliding);
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_FORCE:
			{
				if (!(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingDone))
				{
					ZbAnanlogInput_EnableAcq(TRUE);
					DELAY_US(1000L * DELAY_OP_COMPLETE);
					ZbAnanlogInput_EnableAcq(FALSE);
				}
				CLAMP_ReadForce();
			}
			break;

		case ACT_DBG_READ_RAW_ADC:
			{
				if (!(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingDone))
				{
					ZbAnanlogInput_EnableAcq(TRUE);
					DELAY_US(1000L * DELAY_OP_COMPLETE);
					ZbAnanlogInput_EnableAcq(FALSE);
				}
				CLAMP_ReadADCOffset();
			}
			break;

		case ACT_DBG_READ_LENZE_ERROR:
			{
				MuteRegulator = TRUE;
				DataTable[REG_DRV_ERROR] = CLAMP_ReadError();
				MuteRegulator = FALSE;
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

				CANopen_ClearFault();
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

		case ACT_DBG_READ_LENZE_REG:
			{
				MuteRegulator = TRUE;

				Int32U Value = CLAMP_ReadRegister(DataTable[REG_DBG_CAN_INDEX], DataTable[REG_DBG_CAN_SUBCODE]);
				DEVPROFILE_WriteValue32((pInt16U)DataTable, REG_DBG_READ_REG, Value);

				MuteRegulator = FALSE;
			}
			break;

		case ACT_DBG_WRITE_LENZE_REG:
			{
				MuteRegulator = TRUE;

				Int32U Value = DEVPROFILE_ReadValue32((pInt16U)DataTable, REG_DBG_CAN_DATA);
				CLAMP_WriteRegister(DataTable[REG_DBG_CAN_INDEX], DataTable[REG_DBG_CAN_SUBCODE], Value);

				MuteRegulator = FALSE;
			}
			break;

		case ACT_DBG_BREAK_MAN_RLS_ON:
			CLAMP_BrakeManualRelease(TRUE);
			CLAMP_BrakeAutoControl(FALSE);
			break;

		case ACT_DBG_BREAK_MAN_RLS_OFF:
			CLAMP_BrakeManualRelease(FALSE);
			CLAMP_BrakeAutoControl(FALSE);
			break;

		case ACT_DBG_BREAK_AUTO_CONTROL:
			CLAMP_BrakeAutoControl(TRUE);
			break;

		case ACT_DBG_CON_POWER_SW:
			ZbGPIO_EnablePowerSwitch(TRUE);
			break;

		case ACT_DBG_DISCON_POWER_SW:
			ZbGPIO_EnablePowerSwitch(FALSE);
			break;

		case ACT_DBG_SLS_PUSH_UP:
			ZbGPIO_PneumoPushUp(TRUE);
			break;

		case ACT_DBG_SLS_PUSH_DOWN:
			ZbGPIO_PneumoPushUp(FALSE);
			break;

		case ACT_DBG_SLS_PUSH_OUT:
			ZbGPIO_PneumoPushOut(TRUE);
			break;

		case ACT_DBG_SLS_PUSH_IN:
			ZbGPIO_PneumoPushOut(FALSE);
			break;

		case ACT_DBGCAN_HEADS_DOWN:
			ZbGPIO_HeadsDown(TRUE);
			break;

		case ACT_DBGCAN_HEADS_UP:
			ZbGPIO_HeadsDown(FALSE);
			break;

		case ACT_DBGCAN_BEER_OPEN:
			ZbGPIO_OpenBeerValve(TRUE);
			break;

		case ACT_DBGCAN_BEER_CLOSE:
			ZbGPIO_OpenBeerValve(FALSE);
			break;

		case ACT_DBGCAN_CO2_OPEN:
			ZbGPIO_OpenCO2Valve(TRUE);
			break;

		case ACT_DBGCAN_CO2_CLOSE:
			ZbGPIO_OpenCO2Valve(FALSE);
			break;

		case ACT_DBGCAN_ASYNC_MTR_START:
			CONTROL_ResetScopes();
			PI130_StartMotor(TRUE);
			break;

		case ACT_DBGCAN_ASYNC_MTR_STOP:
			CONTROL_ResetScopes();
			PI130_StartMotor(FALSE);
			break;

		case ACT_DBG_CAN_EXEC_POURING:
			CONTROL_SetMasterState(MS_RequireStart);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

Int16U CONTROL_FlipWord(Int16U Word)
{
	return (Word >> 8) | (Word << 8);
}
// ----------------------------------------

void CONTROL_PDOMonitor()
{
	Int32U ValH = 0, ValL = 0;
	Boolean PdoReady = CANopen_PDOMonitor(&DEVICE_CANopen_Interface, &ValH, &ValL);

	if(PdoReady)
	{
		Int16U Torque = CONTROL_FlipWord(ValH >> 16);
		Int16U PosL = CONTROL_FlipWord(ValL >> 16);
		Int16U PosH = CONTROL_FlipWord(ValL & 0xFFFF);

		DataTable[140] = 1;
		DataTable[141] = 0xFFFF & CONTROL_HSCounter;
		DataTable[142] = Torque;
		DataTable[143] = PosL;
		DataTable[144] = PosH;

		if(CONTROL_Values_Counter < VALUES_x_SIZE)
		{
			CONTROL_Values_Time[CONTROL_Values_Counter] = 0xFFFF & CONTROL_HSCounter;
			CONTROL_Values_Torque[CONTROL_Values_Counter] = Torque;
			CONTROL_Values_Position[CONTROL_Values_Counter] = (Int32U)PosH << 16 | PosL;
			++CONTROL_Values_Counter;
		}
		else
			ZwTimer_StopT1();
	}
	else
	{
		DataTable[140] = 0;
		DataTable[141] = 0;
		DataTable[142] = 0;
		DataTable[143] = 0;
		DataTable[144] = 0;
	}
}
// ----------------------------------------

static void CONTROL_SetMasterState(MasterState NewState)
{
	// Set new state
	CONTROL_MasterState = NewState;
	DataTable[REG_MASTER_STATE] = NewState;
}
// ----------------------------------------

void CONTROL_ProcessMasterEvents()
{
	static Int64U Timeout = 0;
	static Boolean SensorStopPouring = FALSE;
	static Int16S StopBeerDelay = 0;

	switch(CONTROL_MasterState)
	{
		case MS_RequireStart:
			SensorStopPouring = DataTable[REG_STOP_BEER_BY_SENSOR];
			StopBeerDelay = (Int16S)DataTable[REG_BEER_TO_H_UP_PAUSE] / 100;

			Timeout = CONTROL_TimeCounter + DataTable[REG_H_DOWN_TO_CO2_PAUSE] / 100;
			ZbGPIO_HeadsDown(TRUE);
			CONTROL_SetMasterState(MS_PreCO2Pause);
			break;

		case MS_PreCO2Pause:
			if(CONTROL_TimeCounter > Timeout)
			{
				Timeout = CONTROL_TimeCounter + DataTable[REG_CO2_POURING_TIME] / 100;
				ZbGPIO_OpenCO2Valve(TRUE);
				CONTROL_SetMasterState(MS_PouringCO2);
			}
			break;

		case MS_PouringCO2:
			if(CONTROL_TimeCounter > Timeout)
			{
				Timeout = CONTROL_TimeCounter + DataTable[REG_CO2_TO_BEER_PAUSE] / 100;
				ZbGPIO_OpenCO2Valve(FALSE);
				CONTROL_SetMasterState(MS_PostCO2Pause);
			}
			break;

		case MS_PostCO2Pause:
			if(CONTROL_TimeCounter > Timeout)
			{
				Timeout = CONTROL_TimeCounter + DataTable[REG_BEER_POURING_TIME] / 100;
				ZbGPIO_OpenBeerValve(TRUE);
				CONTROL_SetMasterState(MS_PouringBeer);
			}
			break;

		case MS_PouringBeer:
			if((SensorStopPouring) || (!SensorStopPouring && CONTROL_TimeCounter > Timeout))
			{
				if(StopBeerDelay < 0)
				{
					Timeout = CONTROL_TimeCounter - StopBeerDelay;
					ZbGPIO_HeadsDown(FALSE);
					CONTROL_SetMasterState(MS_HeadsUpWBeer);
				}
				else
				{
					Timeout = CONTROL_TimeCounter + StopBeerDelay;
					ZbGPIO_OpenBeerValve(FALSE);
					CONTROL_SetMasterState(MS_PreHeadsUp);
				}
			}
			break;

		case MS_PreHeadsUp:
			if(CONTROL_TimeCounter > Timeout)
			{
				ZbGPIO_HeadsDown(FALSE);
				CONTROL_SetMasterState(MS_None);
			}
			break;

		case MS_HeadsUpWBeer:
			if(CONTROL_TimeCounter > Timeout)
			{
				ZbGPIO_OpenBeerValve(FALSE);
				CONTROL_SetMasterState(MS_None);
			}
			break;

		default:
			break;
	}

}
// ----------------------------------------
