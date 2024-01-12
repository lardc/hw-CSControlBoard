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
#include "TRM101.h"
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
volatile Int64U CONTROL_TimeCounter = 0, Timeout;
volatile Int32U FanTimeout = 0;
volatile DeviceState CONTROL_State = DS_None;
//
#pragma DATA_SECTION(CONTROL_Values_1, "data_mem");
Int16U CONTROL_Values_1[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_2, "data_mem");
Int16U CONTROL_Values_2[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_3, "data_mem");
Int16U CONTROL_Values_3[VALUES_x_SIZE];
//
#pragma DATA_SECTION(CONTROL_Values_1_32, "data_mem");
Int32U CONTROL_Values_1_32[VALUES_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_2_32, "data_mem");
Int32U CONTROL_Values_2_32[VALUES_x_SIZE];
//
// Extended data logger
#pragma DATA_SECTION(CONTROL_Values_SubState, "data_mem");
Int16U CONTROL_Values_SubState[VALUES_XLOG_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_Force, "data_mem");
Int16U CONTROL_Values_Force[VALUES_XLOG_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_Error, "data_mem");
Int16U CONTROL_Values_Error[VALUES_XLOG_x_SIZE];
#pragma DATA_SECTION(CONTROL_Values_TorqueLimit, "data_mem");
Int16U CONTROL_Values_TorqueLimit[VALUES_XLOG_x_SIZE];
//
volatile Int16U CONTROL_Values_Counter = 0;
volatile Int16U CONTROL_Values_XLogCounter = 0;
//
// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;


// Forward functions
//
static void CONTROL_HandleClampActions();
static void CONTROL_UpdateTemperatureFeedback();
static void CONTROL_HandleFanControl();
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
static void CONTROL_ResetScopes();
Boolean CONTROL_SlidingSensorOK();
Boolean CONTROL_PressureOK();


// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes_16[EP_COUNT_16] = {EP16_Data_ForceActual, EP16_Data_ForceDesired, EP16_Data_ForceError,
			EP16_XLog_SubState, EP16_XLog_Force, EP16_XLog_Error, EP16_XLog_TorqueLimit};
	Int16U EPSized_16[EP_COUNT_16] = {VALUES_x_SIZE, VALUES_x_SIZE, VALUES_x_SIZE, VALUES_XLOG_x_SIZE,
			VALUES_XLOG_x_SIZE, VALUES_XLOG_x_SIZE, VALUES_XLOG_x_SIZE};
	pInt16U EPCounters_16[EP_COUNT_16] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_XLogCounter,
			(pInt16U)&CONTROL_Values_XLogCounter, (pInt16U)&CONTROL_Values_XLogCounter,
			(pInt16U)&CONTROL_Values_XLogCounter};
	pInt16U EPDatas_16[EP_COUNT_16] = {CONTROL_Values_1, CONTROL_Values_2, CONTROL_Values_3, CONTROL_Values_SubState,
			CONTROL_Values_Force, CONTROL_Values_Error, CONTROL_Values_TorqueLimit};

	// Variables for endpoint configuration
	Int16U EPIndexes_32[EP_COUNT_32] = {EP32_Data_CtrlIncrements, EP32_Data_Position};
	Int16U EPSized_32[EP_COUNT_32] = {VALUES_x_SIZE, VALUES_x_SIZE};
	pInt16U EPCounters_32[EP_COUNT_32] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter};
	pInt16U EPDatas_32[EP_COUNT_32] = {(pInt16U)CONTROL_Values_1_32, (pInt16U)CONTROL_Values_2_32};

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

		// Heating system init
		if(DataTable[REG_USE_HEATING])
		{
			TRMError dummy_error;

			// Terminate heating for sure
			TRM_Stop(TRM_CH1_ADDR, &dummy_error);
			TRM_Stop(TRM_CH2_ADDR, &dummy_error);
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

	// Update temperature feedback
	CONTROL_UpdateTemperatureFeedback();

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

void inline CONTROL_RequestDPC(FUNC_AsyncDelegate Action)
{
	DPCDelegate = Action;
}
// ----------------------------------------

void inline CONTROL_RequestClampRelease()
{
	if(CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready || CONTROL_State == DS_Halt
			|| CONTROL_State == DS_None)
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

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_NotifyCANbFault, "ramfuncs");
#endif
void CONTROL_NotifyCANbFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANbFault(Flag);
}
// ----------------------------------------

void CONTROL_NotifyCANopenFault()
{
	DataTable[REG_FAULT_REASON] = FAULT_CANOPEN;
	CONTROL_SetDeviceState(DS_Fault);
}
// ----------------------------------------

static void CONTROL_UpdateTemperatureFeedback()
{
	if(DataTable[REG_USE_HEATING] && !DataTable[REG_DBG_PAUSE_T_FEEDBACK])
		TempFb_UpdateTemperatureFeedback(CONTROL_TimeCounter);
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
	if(NewState == DS_Clamping || NewState == DS_ClampingUpdate)
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

	if(err)
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

static void CONTROL_HandleFanControl()
{
	ZbGPIO_SwitchFan((FanTimeout > 0 || HeatingActive) ? TRUE : FALSE);
	if(FanTimeout > 0)
		--FanTimeout;
}
// ----------------------------------------

static void CONTROL_HandleClampActions()
{
	static Int64U ClampingDoneCounterCopy = 0;

	// Extended data logging
	CLAMPCTRL_XLog(CONTROL_State);

	// Disable any actions with motor driver
	if(MuteRegulator)
		return;

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
				if(!DataTable[REG_NO_HALT_ON_QUICK_STOP] && CLAMP_IsQSPActive())
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
			if(CLAMP_IsHomingDone())
			{
				CLAMP_CompleteOperation(FALSE);

				// Go to initial position
				CLAMP_SpeedTorqueLimits(DataTable[REG_POSITION_SPEED_LIMIT], DataTable[REG_POSITION_TORQUE_LIMIT]);
				CLAMP_GoToPosition_mm(FALSE, DataTable[REG_CLAMPING_RLS_POS]);
				CONTROL_SetDeviceState(DS_Position);
			}
			break;

		case DS_Position:
			if(CLAMP_IsTargetReached())
			{
				CLAMP_CompleteOperation(TRUE);
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;

		case DS_Clamping:
			if(CLAMPCTRL_IsClampingDone())
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
				if((EnableAutoRelease && (CONTROL_TimeCounter - ClampingDoneCounterCopy > AutoReleaseTimeoutTicks))
						|| !CONTROL_PressureOK())
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
			if(CLAMPCTRL_IsClampingReleaseDone())
			{
				CLAMP_CompleteOperation(TRUE);
				CONTROL_SetDeviceState(DS_Ready);
			}
			break;

		case DS_Sliding:
			{
				if(CONTROL_TimeCounter < Timeout)
				{
					if(CONTROL_SlidingSensorOK())
						++SlidingCounter;
					else
						SlidingCounter = 0;

					if(SlidingCounter > SLS_BOUNCE_COUNTER)
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
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if(CONTROL_CheckLenzeError())
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
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if(CONTROL_CheckLenzeError()
						|| ((CONTROL_State == DS_None || CONTROL_State == DS_Halt) && !CLAMP_IsHomingPosAvailable()))
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					if(DataTable[REG_CUSTOM_POS] <= DataTable[REG_ALLOWED_MOVE])
					{
						DataTable[REG_PROBLEM] = PROBLEM_NONE;
						CLAMP_SpeedTorqueLimits(DataTable[REG_POSITION_SPEED_LIMIT],
								DataTable[REG_POSITION_TORQUE_LIMIT]);
						CLAMP_GoToPosition_mm(TRUE, DataTable[REG_CUSTOM_POS]);
						CONTROL_SetDeviceState(DS_Position);
					}
					else
						*UserError = ERR_PARAMETER_OUT_OF_RNG;
				}
			}
			else
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_START_CLAMPING:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready)
			{
				if(!CONTROL_PressureOK())
				{
					*UserError = ERR_NO_AIR_PRESSURE;
				}
				else if(!CONTROL_SlidingSensorOK())
				{
					*UserError = ERR_SLIDING_SYSTEM;
				}
				else if(CONTROL_CheckLenzeError()
						|| ((CONTROL_State == DS_None || CONTROL_State == DS_Halt) && !CLAMP_IsHomingPosAvailable()))
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					AutoReleaseTimeoutTicks = ((Int32U)DataTable[REG_MAX_CONT_FORCE_TIMEOUT] * CS_MONITORING_FREQ)
							/ 1000;
					EnableAutoRelease =
							(DataTable[REG_FORCE_VAL] >= DataTable[REG_MAX_CONT_FORCE]
									&& DataTable[REG_USE_CLAMP_BREAK] == 0) ? TRUE : FALSE;

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
			if(CONTROL_State == DS_ClampingDone)
			{
				// prevent interrupt by regulator
				MuteRegulator = TRUE;

				if(CONTROL_CheckLenzeError())
				{
					*UserError = ERR_DEVICE_NOT_READY;
				}
				else
				{
					AutoReleaseTimeoutTicks = ((Int32U)DataTable[REG_MAX_CONT_FORCE_TIMEOUT] * CS_MONITORING_FREQ)
							/ 1000;
					EnableAutoRelease =
							(DataTable[REG_FORCE_VAL] >= DataTable[REG_MAX_CONT_FORCE]
									&& DataTable[REG_USE_CLAMP_BREAK] == 0) ? TRUE : FALSE;

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
			if(CONTROL_State == DS_ClampingDone || CONTROL_State == DS_Ready || CONTROL_State == DS_Halt
					|| CONTROL_State == DS_None)
			{
				// prevent interrupt by regulator
				MuteRegulator = TRUE;

				if(CONTROL_CheckLenzeError())
					*UserError = ERR_DEVICE_NOT_READY;
				else
					CONTROL_RequestClampRelease();

				// resume regulator
				MuteRegulator = FALSE;
			}
			else if(CONTROL_State != DS_ClampingRelease)
				*UserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_HALT:
			{
				DINT;
				CLAMP_CompleteOperation(TRUE);
				CONTROL_SetDeviceState(DS_Halt);
				EINT;
			}
			break;

		case ACT_SLIDING_PUSH_OUT:
			{
				if(CONTROL_State == DS_Ready)
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
				if(CONTROL_State == DS_Ready)
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
				if(!(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingDone))
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
				if(!(CONTROL_State == DS_Clamping || CONTROL_State == DS_ClampingDone))
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

		case ACT_SET_TEMPERATURE:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					if(DataTable[REG_TEMP_SETPOINT] < TRM_TEMP_THR)
					{
						TRM_SetTemp(TRM_CH1_ADDR, TRM_ROOM_TEMP, &error);
						if(error == TRME_None) TRM_SetTemp(TRM_CH2_ADDR, TRM_ROOM_TEMP, &error);
						if(error == TRME_None) TRM_Stop(TRM_CH1_ADDR, &error);
						if(error == TRME_None) TRM_Stop(TRM_CH2_ADDR, &error);

						HeatingActive = FALSE;
					}
					else
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None) TRM_SetTemp(TRM_CH2_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if(error == TRME_None) TRM_Start(TRM_CH1_ADDR, &error);
						if(error == TRME_None) TRM_Start(TRM_CH2_ADDR, &error);

						HeatingActive = TRUE;
					}

					if(error != TRME_None)
					{
						DataTable[REG_TRM_ERROR] = error;
						DataTable[REG_FAULT_REASON] = FAULT_TRM;
						CONTROL_SetDeviceState(DS_Fault);
						*UserError = ERR_TRM_COMM_ERR;
					}

					// resume regulator
					MuteRegulator = FALSE;
				}
				else
				{
					DataTable[REG_TEMP_CH1] = DataTable[REG_TEMP_SETPOINT];
					DataTable[REG_TEMP_CH2] = DataTable[REG_TEMP_SETPOINT];
				}
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

		case ACT_DBG_WRITE_DAC_RAW:
			if(CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready
					|| CONTROL_State == DS_Fault || CONTROL_State == DS_Disabled)
			{
				if(DataTable[REG_DBG_PAUSE_T_FEEDBACK] && DataTable[REG_USE_HEATING])
					(DataTable[REG_DBG_TEMP_CH_INDEX] == 1) ?
							ZbDAC_WriteA(DataTable[REG_DBG_TEMP_CH_DATA]) :
							ZbDAC_WriteB(DataTable[REG_DBG_TEMP_CH_DATA]);
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_DBG_WRITE_DAC_TEMP:
			if((CONTROL_State == DS_None || CONTROL_State == DS_Halt || CONTROL_State == DS_Ready
					|| CONTROL_State == DS_Fault || CONTROL_State == DS_Disabled))
			{
				if(DataTable[REG_DBG_PAUSE_T_FEEDBACK] && DataTable[REG_USE_HEATING])
				{
					_iq temp = _FPtoIQ2(DataTable[REG_DBG_TEMP_CH_DATA], 10);
					(DataTable[REG_DBG_TEMP_CH_INDEX] == 1) ? AnalogOutput_SetCH1(temp) : AnalogOutput_SetCH2(temp);
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			else
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_DBG_READ_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					_iq temp =
							(DataTable[REG_DBG_TEMP_CH_INDEX] == 1) ?
									TempFb_GetTemperatureCH1() : TempFb_GetTemperatureCH2();
					DataTable[REG_DBG_TEMP] = _IQmpyI32int(temp, 10);
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_TEMP_RAW:
			{
				if(DataTable[REG_USE_HEATING])
					DataTable[REG_DBG_TEMP_RAW] =
							(DataTable[REG_DBG_TEMP_CH_INDEX] == 1) ? ZbTh_ReadSEN1() : ZbTh_ReadSEN2();
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_TRM_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

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

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					DataTable[REG_TRM_DATA] = TRM_ReadPower(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

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

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRM_Start(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

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

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRM_Stop(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
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

		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------
