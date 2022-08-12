// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
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

typedef enum __MasterStatePouring
{
	MSP_None = 0,
	MSP_RequirePouringStart = 1,
	MSP_PreCO2Pause = 2,
	MSP_PouringCO2 = 3,
	MSP_PostCO2Pause = 4,
	MSP_PouringBeer = 5,
	MSP_PreHeadsUp = 6,
	MSP_HeadsUpWBeer = 7

} MasterStatePouring;

typedef enum __MasterStateSeaming
{
	MSS_None = 0,
	MSS_RequireHoming = 1,
	MSS_WaitHoming = 2,
	MSS_RequireSeamingStart = 3,
	MSS_WaitSeamerPushUp = 4,
	MSS_WaitSpindleSpinUp = 5,
	MSS_SeamingStep1Fast = 6,
	MSS_SeamingStep1 = 7,
	MSS_PostSeamingStep1 = 8,
	MSS_SeamingStep2Fast = 9,
	MSS_SeamingStep2 = 10,
	MSS_PostSeamingStep2 = 11,
	MSS_MoveToZero = 12,
	MSS_StopSpindle = 13,
	MSS_WaitStopSpindle = 14

} MasterStateSeaming;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter, CONTROL_HSCounter;
extern volatile DeviceState CONTROL_State;
extern volatile Int16U CONTROL_BootLoaderRequest;
extern volatile Int16U CONTROL_Values_Counter;

#define VALUES_RX_SIZE	32
extern volatile Int16U CONTROL_RS485_Rx[];
extern volatile Int16U CONTROL_Rx_Counter;

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
