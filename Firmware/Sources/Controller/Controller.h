﻿// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "OWENProtocol.h"
#include "CANopenTypes.h"
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "Clamp.h"

// Types
//
typedef enum __DeviceState
{
	DS_None	= 0,
	DS_Fault = 1,
	DS_Disabled = 2,
	DS_Ready = 3,
	DS_Halt = 4,
	DS_Homing = 5,
	DS_Position = 6,
	DS_Clamping = 7,
	DS_ClampingDone = 8,
	DS_ClampingUpdate = 9,
	DS_ClampingRelease = 10,
	DS_Sliding = 11
} DeviceState;

typedef enum __MasterState
{
	MS_None = 0,
	MS_RequireStart = 1,
	MS_PreCO2Pause = 2,
	MS_PouringCO2 = 3,
	MS_PostCO2Pause = 4,
	MS_PouringBeer = 5
} MasterState;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter, CONTROL_HSCounter;
extern volatile DeviceState CONTROL_State;
extern volatile Int16U CONTROL_BootLoaderRequest;
extern volatile Int16U CONTROL_Values_Counter;

// Functions
//
// Initialize controller
void CONTROL_Init(Boolean BadClockDetected);
// Do background idle operation
void CONTROL_Idle();
// Update low-priority tasks
void CONTROL_UpdateLow();
// Notify that CANa system fault occurs
void CONTROL_NotifyCANaFault(ZwCAN_SysFlags Flag);
// Notify that CANa system fault occurs
void CONTROL_NotifyCANbFault(ZwCAN_SysFlags Flag);
// Notify about high-level CANopen fault
void CONTROL_NotifyCANopenFault(CANopenErrCode ErrorCode, Int16U Index, Int16U SubIndex, Int32U Value);
void CONTROL_PDOMonitor();

#endif // __CONTROLLER_H
