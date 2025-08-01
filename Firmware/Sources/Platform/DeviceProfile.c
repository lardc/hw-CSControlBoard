﻿// -----------------------------------------
// Device profile
// ----------------------------------------

// Header
#include "DeviceProfile.h"
//
// Includes
#include "SysConfig.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "ZwDSP.h"
#include "DataTable.h"
#include "Controller.h"
#include "Constraints.h"


// Types
//
typedef struct __EPState
{
	Int16U Size;
	Int16U ReadCounter;
	Int16U LastReadCounter;
	pInt16U pDataCounter;
	pInt16U Data;
} EPState, *pEPState;
//
typedef struct __EPStates_16
{
	EPState EPs[EP_COUNT_16];
} EPStates_16, *pEPStates_16;
//
typedef struct __EPStates_32
{
	EPState EPs[EP_COUNT_32];
} EPStates_32, *pEPStates_32;


// Variables
//
SCCI_Interface DEVICE_RS232_Interface;
BCCI_Interface DEVICE_CAN_Interface;
CANopen_Interface DEVICE_CANopen_Interface;
//
static SCCI_IOConfig RS232_IOConfig;
static BCCI_IOConfig CAN_IOConfig;
static CANopen_IOConfig CANopenBus_IOConfig;
static xCCI_ServiceConfig X_ServiceConfig;
static xCCI_FUNC_CallbackAction ControllerDispatchFunction;
static EPStates_16 RS232_EPState_16, CAN_EPState_16;
static EPStates_32 RS232_EPState_32;
static Boolean UnlockedForNVWrite = FALSE;
//
static volatile Boolean *MaskChangesFlag;


// Forward functions
//
void DEVPROFILE_CANopen_NetworkFail();
static void DEVPROFILE_FillWRPartDefault();
static Boolean DEVPROFILE_Validate32(Int16U Address, Int32U Data);
static Boolean DEVPROFILE_Validate16(Int16U Address, Int16U Data);
static Boolean DEVPROFILE_DispatchAction(Int16U ActionID, pInt16U UserError);
static Int16U DEVPROFILE_CallbackReadX16(Int16U Endpoint, pInt16U *Buffer, Boolean Streamed,
									     Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize);
static Int16U DEVPROFILE_CallbackReadX32(Int16U Endpoint, pInt32U *Buffer, Boolean Streamed,
									     Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize);


// Functions
//
void DEVPROFILE_Init(xCCI_FUNC_CallbackAction SpecializedDispatch, volatile Boolean *MaskChanges)
{
	// Save values
	ControllerDispatchFunction = SpecializedDispatch;
	MaskChangesFlag = MaskChanges;

	// Init interface
	RS232_IOConfig.IO_SendArray16 = &ZwSCIb_SendArray16;
	RS232_IOConfig.IO_ReceiveArray16 = &ZwSCIb_ReceiveArray16;
	RS232_IOConfig.IO_GetBytesToReceive = &ZwSCIb_GetBytesToReceive;
	RS232_IOConfig.IO_ReceiveByte = &ZwSCIb_ReceiveChar;
	//
	CAN_IOConfig.IO_SendMessage = &ZwCANa_SendMessage;
	CAN_IOConfig.IO_SendMessageEx = &ZwCANa_SendMessageEx;
	CAN_IOConfig.IO_GetMessage = &ZwCANa_GetMessage;
	CAN_IOConfig.IO_IsMessageReceived = &ZwCANa_IsMessageReceived;
	CAN_IOConfig.IO_ConfigMailbox = &ZwCANa_ConfigMailbox;
	//
	CANopenBus_IOConfig.IO_ConfigMailbox = &ZwCANb_ConfigMailbox;
	CANopenBus_IOConfig.IO_GetMessage = &ZwCANb_GetMessage;
	CANopenBus_IOConfig.IO_GetTimeStamp = &ZwCANb_GetTimeStamp;
	CANopenBus_IOConfig.IO_IsMessageReceived = &ZwCANb_IsMessageReceived;
	CANopenBus_IOConfig.IO_SendMessage = &ZwCANb_SendMessage;
	CANopenBus_IOConfig.IO_ClearMailbox = &ZwCANb_CancelMessage;

	// Init service
	X_ServiceConfig.Read32Service = &DEVPROFILE_ReadValue32;
	X_ServiceConfig.Write32Service = &DEVPROFILE_WriteValue32;
	X_ServiceConfig.UserActionCallback = &DEVPROFILE_DispatchAction;
	X_ServiceConfig.ValidateCallback16 = &DEVPROFILE_Validate16;
	X_ServiceConfig.ValidateCallback32 = &DEVPROFILE_Validate32;

	// Init interface driver
	SCCI_Init(&DEVICE_RS232_Interface, &RS232_IOConfig, &X_ServiceConfig, (pInt16U)DataTable,
			  DATA_TABLE_SIZE, SCCI_TIMEOUT_TICKS, &RS232_EPState_16, &RS232_EPState_32);
	BCCI_Init(&DEVICE_CAN_Interface, &CAN_IOConfig, &X_ServiceConfig, (pInt16U)DataTable,
			  DATA_TABLE_SIZE, &CAN_EPState_16);
	CANopen_Init(&DEVICE_CANopen_Interface, &CANopenBus_IOConfig, &DEVPROFILE_CANopen_NetworkFail);

	// Set write protection
	SCCI_AddProtectedArea(&DEVICE_RS232_Interface, DATA_TABLE_WP_START, DATA_TABLE_SIZE - 1);
	BCCI_AddProtectedArea(&DEVICE_CAN_Interface, DATA_TABLE_WP_START, DATA_TABLE_SIZE - 1);
}
// ----------------------------------------

void DEVPROFILE_InitEPService16(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas)
{
	Int16U i;

	for(i = 0; i < EP_COUNT_16; ++i)
	{
		RS232_EPState_16.EPs[i].Size = Sizes[i];
		RS232_EPState_16.EPs[i].pDataCounter = Counters[i];
		RS232_EPState_16.EPs[i].Data = Datas[i];

		CAN_EPState_16.EPs[i].Size = Sizes[i];
		CAN_EPState_16.EPs[i].pDataCounter = Counters[i];
		CAN_EPState_16.EPs[i].Data = Datas[i];

		RS232_EPState_16.EPs[i].ReadCounter = RS232_EPState_16.EPs[i].LastReadCounter = 0;
		CAN_EPState_16.EPs[i].ReadCounter = CAN_EPState_16.EPs[i].LastReadCounter = 0;

		SCCI_RegisterReadEndpoint16(&DEVICE_RS232_Interface, Indexes[i], &DEVPROFILE_CallbackReadX16);
		BCCI_RegisterReadEndpoint16(&DEVICE_CAN_Interface, Indexes[i], &DEVPROFILE_CallbackReadX16);
	}
}
// ----------------------------------------

void DEVPROFILE_InitEPService32(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas)
{
	Int16U i;

	for(i = 0; i < EP_COUNT_32; ++i)
	{
		RS232_EPState_32.EPs[i].Size = Sizes[i];
		RS232_EPState_32.EPs[i].pDataCounter = Counters[i];
		RS232_EPState_32.EPs[i].Data = Datas[i];

		RS232_EPState_32.EPs[i].ReadCounter = RS232_EPState_32.EPs[i].LastReadCounter = 0;

		SCCI_RegisterReadEndpoint32(&DEVICE_RS232_Interface, Indexes[i], &DEVPROFILE_CallbackReadX32);
	}
}
// ----------------------------------------

void DEVPROFILE_ProcessRequests()
{
	// Handle interface requests
	SCCI_Process(&DEVICE_RS232_Interface, CONTROL_TimeCounter, *MaskChangesFlag);

	// Handle interface requests
	BCCI_Process(&DEVICE_CAN_Interface, *MaskChangesFlag);
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(DEVPROFILE_ResetEPReadState, "ramfuncs");
#endif
void DEVPROFILE_ResetEPReadState()
{
	Int16U i;

	for(i = 0; i < EP_COUNT_16; ++i)
	{
		RS232_EPState_16.EPs[i].ReadCounter = 0;
		CAN_EPState_16.EPs[i].ReadCounter = 0;
		RS232_EPState_16.EPs[i].LastReadCounter = 0;
		CAN_EPState_16.EPs[i].LastReadCounter = 0;
	}

	for(i = 0; i < EP_COUNT_32; ++i)
	{
		RS232_EPState_32.EPs[i].ReadCounter = 0;
		RS232_EPState_32.EPs[i].LastReadCounter = 0;
	}
}
// ----------------------------------------

void DEVPROFILE_CANopen_NetworkFail()
{
	CONTROL_NotifyCANopenFault();
}
// ----------------------------------------

void DEVPROFILE_ResetControlSection()
{
	DT_ResetWRPart(&DEVPROFILE_FillWRPartDefault);
}
// ----------------------------------------

void DEVPROFILE_ResetScopes16(Int16U ResetPosition)
{
	Int16U i;

	for(i = 0; i < EP_COUNT_16; ++i)
	{
		*(RS232_EPState_16.EPs[i].pDataCounter) = ResetPosition;
		*(CAN_EPState_16.EPs[i].pDataCounter) = ResetPosition;

		MemZero16(RS232_EPState_16.EPs[i].Data, RS232_EPState_16.EPs[i].Size);
		MemZero16(CAN_EPState_16.EPs[i].Data, CAN_EPState_16.EPs[i].Size);
	}
}
// ----------------------------------------

void DEVPROFILE_ResetScopes32(Int16U ResetPosition)
{
	Int16U i;

	for(i = 0; i < EP_COUNT_32; ++i)
	{
		*(RS232_EPState_32.EPs[i].pDataCounter) = ResetPosition;
		MemZero32((pInt32U)RS232_EPState_32.EPs[i].Data, RS232_EPState_32.EPs[i].Size);
	}
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(DEVPROFILE_NotifyCANaFault, "ramfuncs");
#endif
void DEVPROFILE_NotifyCANaFault(ZwCAN_SysFlags Flag)
{
	// Update error counter
	if(Flag & BOIM)
		DataTable[REG_CANA_BUSOFF_COUNTER]++;

	// Cancel messages in the case of bus fault
	if(Flag & (EPIM | BOIM))
		ZwCANa_CancelAllMessages();
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(DEVPROFILE_NotifyCANbFault, "ramfuncs");
#endif
void DEVPROFILE_NotifyCANbFault(ZwCAN_SysFlags Flag)
{
	// Update error counter
	if(Flag & BOIM)
		DataTable[REG_CANB_BUSOFF_COUNTER]++;

	// Cancel messages in the case of bus fault
	if(Flag & (EPIM | BOIM))
		ZwCANb_CancelAllMessages();
}
// ----------------------------------------

void DEVPROFILE_UpdateCANDiagStatus()
{
	CANDiagnosticInfo info;

	info = ZwCANa_GetDiagnosticInfo();
	DEVPROFILE_WriteValue32((pInt16U)DataTable, REG_CANA_STATUS_REG, info.Status);
	DataTable[REG_CANA_DIAG_TEC] = info.TEC;
	DataTable[REG_CANA_DIAG_REC] = info.REC;

	info = ZwCANb_GetDiagnosticInfo();
	DEVPROFILE_WriteValue32((pInt16U)DataTable, REG_CANB_STATUS_REG, info.Status);
	DataTable[REG_CANB_DIAG_TEC] = info.TEC;
	DataTable[REG_CANB_DIAG_REC] = info.REC;
}
// ----------------------------------------

Int32U DEVPROFILE_ReadValue32(pInt16U pTable, Int16U Index)
{
	return pTable[Index] | (((Int32U)(pTable[Index + 1])) << 16);
}
// ----------------------------------------

void DEVPROFILE_WriteValue32(pInt16U pTable, Int16U Index, Int32U Data)
{
	pTable[Index] = Data & 0x0000FFFF;
	pTable[Index + 1] = Data >> 16;
}
// ----------------------------------------

void DEVPROFILE_FillNVPartDefault()
{
	Int16U i;

	// Write default values to data table
	for(i = 0; i < DATA_TABLE_NV_SIZE; ++i)
		DataTable[DATA_TABLE_NV_START + i] = NVConstraint[i].Default;
}
// ----------------------------------------

static void DEVPROFILE_FillWRPartDefault()
{
	Int16U i;

	// Write default values to data table
	for(i = 0; i < (DATA_TABLE_WP_START - DATA_TABLE_WR_START); ++i)
		DataTable[DATA_TABLE_WR_START + i] = VConstraint[i].Default;
}
// ----------------------------------------

static Boolean DEVPROFILE_Validate16(Int16U Address, Int16U Data)
{
	if(ENABLE_LOCKING && !UnlockedForNVWrite && (Address < DATA_TABLE_WR_START))
		return FALSE;

	if(Address < DATA_TABLE_WR_START)
	{
		if(Data < NVConstraint[Address - DATA_TABLE_NV_START].Min || Data > NVConstraint[Address - DATA_TABLE_NV_START].Max)
			return FALSE;
	}
	else if(Address < DATA_TABLE_WP_START)
	{
		Int16U MaxValue = VConstraint[Address - DATA_TABLE_WR_START].Max;
		if(Address == REG_FORCE_VAL && DataTable[REG_MAX_ALLOWED_FORCE])
			MaxValue = DataTable[REG_MAX_ALLOWED_FORCE];

		if(Data < VConstraint[Address - DATA_TABLE_WR_START].Min || Data > MaxValue)
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

static Boolean DEVPROFILE_Validate32(Int16U Address, Int32U Data)
{
	if(ENABLE_LOCKING && !UnlockedForNVWrite && (Address < DATA_TABLE_WR_START))
		return FALSE;

	return FALSE;
}
// ----------------------------------------

static Boolean DEVPROFILE_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_SAVE_TO_ROM:
			{
				if(ENABLE_LOCKING && !UnlockedForNVWrite)
					*UserError = ERR_WRONG_PWD;
				else
					DT_SaveNVPartToEPROM();
			}
			break;
		case ACT_RESTORE_FROM_ROM:
			{
				if(ENABLE_LOCKING && !UnlockedForNVWrite)
					*UserError = ERR_WRONG_PWD;
				else
					DT_RestoreNVPartFromEPROM();
			}
			break;
		case ACT_RESET_TO_DEFAULT:
			{
				if(ENABLE_LOCKING && !UnlockedForNVWrite)
					*UserError = ERR_WRONG_PWD;
				else
					DT_ResetNVPart(&DEVPROFILE_FillNVPartDefault);
			}
			break;
		case ACT_LOCK_NV_AREA:
			UnlockedForNVWrite = FALSE;
			break;
		case ACT_UNLOCK_NV_AREA:
			if(DataTable[REG_PWD_1] == UNLOCK_PWD_1 &&
				DataTable[REG_PWD_2] == UNLOCK_PWD_2 &&
				DataTable[REG_PWD_3] == UNLOCK_PWD_3 &&
				DataTable[REG_PWD_4] == UNLOCK_PWD_4)
			{
				UnlockedForNVWrite = TRUE;
				DataTable[REG_PWD_1] = 0;
				DataTable[REG_PWD_2] = 0;
				DataTable[REG_PWD_3] = 0;
				DataTable[REG_PWD_4] = 0;
			}
			else
				*UserError = ERR_WRONG_PWD;
			break;
		case ACT_BOOT_LOADER_REQUEST:
			CONTROL_BootLoaderRequest = BOOT_LOADER_REQUEST;
			break;
		default:
			return (ControllerDispatchFunction) ? ControllerDispatchFunction(ActionID, UserError) : FALSE;
	}

	return TRUE;
}
// ----------------------------------------

static Int16U DEVPROFILE_CallbackReadX16(Int16U Endpoint, pInt16U *Buffer, Boolean Streamed,
									     Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize)
{
	Int16U pLen;
	pEPState epState;
	pEPStates_16 epStates = (pEPStates_16)EPStateAddress;

	// Validate pointer
	if(!epStates)
		return 0;

	// Get endpoint
	epState = &epStates->EPs[Endpoint - 1];

	// Handle transmission repeat
	if(RepeatLastTransmission)
		epState->ReadCounter = epState->LastReadCounter;

	// Write possible content reference
	*Buffer = epState->Data + epState->ReadCounter;

	// Calculate content length
	if(*(epState->pDataCounter) < epState->ReadCounter)
		pLen = 0;
	else
		pLen = *(epState->pDataCounter) - epState->ReadCounter;

	if(!Streamed)
		pLen = (pLen > MaxNonStreamSize) ? MaxNonStreamSize : pLen;

	// Update content state
	epState->LastReadCounter = epState->ReadCounter;
	epState->ReadCounter += pLen;

	return pLen;
}
// ----------------------------------------

static Int16U DEVPROFILE_CallbackReadX32(Int16U Endpoint, pInt32U *Buffer, Boolean Streamed,
									     Boolean RepeatLastTransmission, void *EPStateAddress, Int16U MaxNonStreamSize)
{
	Int16U pLen;
	pEPState epState;
	pEPStates_32 epStates = (pEPStates_32)EPStateAddress;

	// Validate pointer
	if(!epStates)
		return 0;

	// Get endpoint
	epState = &epStates->EPs[Endpoint - 1];

	// Handle transmission repeat
	if(RepeatLastTransmission)
		epState->ReadCounter = epState->LastReadCounter;

	// Write possible content reference
	*Buffer = ((pInt32U)epState->Data) + epState->ReadCounter;

	// Calculate content length
	if(*(epState->pDataCounter) < epState->ReadCounter)
		pLen = 0;
	else
		pLen = *(epState->pDataCounter) - epState->ReadCounter;

	if(!Streamed)
		pLen = (pLen > MaxNonStreamSize) ? MaxNonStreamSize : pLen;

	// Update content state
	epState->LastReadCounter = epState->ReadCounter;
	epState->ReadCounter += pLen;

	return pLen;
}

// No more
