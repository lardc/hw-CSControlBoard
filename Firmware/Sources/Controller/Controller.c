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
//

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
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
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
		if (DataTable[REG_USE_HEATING])
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
	return (DataTable[REG_USE_SLIDING_SENSOR]) ? ZbGPIO_GetS3State() : TRUE;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(CONTROL_UpdateLow, "ramfuncs");
#endif
void CONTROL_UpdateLow()
{
	CONTROL_HandleFanControl();
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
	if (NewState == DS_Clamping || NewState == DS_ClampingUpdate)
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
	if (FanTimeout > 0)
		--FanTimeout;
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_HALT:
			{
				DINT;
				CONTROL_SetDeviceState(DS_Halt);
				EINT;
			}
			break;

		case ACT_SET_TEMPERATURE:
			{
				if (DataTable[REG_USE_HEATING])
				{
					TRMError error;

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					if (DataTable[REG_TEMP_SETPOINT] < TRM_TEMP_THR)
					{
						TRM_SetTemp(TRM_CH1_ADDR, TRM_ROOM_TEMP, &error);
						if (error == TRME_None) TRM_Stop(TRM_CH1_ADDR, &error);

						HeatingActive = FALSE;
					}
					else
					{
						TRM_SetTemp(TRM_CH1_ADDR, DataTable[REG_TEMP_SETPOINT], &error);
						if (error == TRME_None) TRM_Start(TRM_CH1_ADDR, &error);

						HeatingActive = TRUE;
					}

					if (error != TRME_None)
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
				if (DataTable[REG_USE_HEATING])
				{
					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

					if (error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_TRM_POWER:
			{
				if (DataTable[REG_USE_HEATING])
				{
					TRMError error;

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					DataTable[REG_TRM_DATA] = TRM_ReadPower(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

					if (error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_TRM_START:
			{
				if (DataTable[REG_USE_HEATING])
				{
					TRMError error;

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRM_Start(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

					if (error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_TRM_STOP:
			{
				if (DataTable[REG_USE_HEATING])
				{
					TRMError error;

					// prevent interrupt by regulator
					MuteRegulator = TRUE;

					TRM_Stop(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					// resume regulator
					MuteRegulator = FALSE;

					if (error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

// No more.
