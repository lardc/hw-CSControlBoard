// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "OWENProtocol.h"
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"
#include "StepperMotor.h"

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
	DS_ClampingRelease = 10
} DeviceState;

typedef enum __DeviceSubState
{
	DSS_None = 0,

	DSS_Com_CheckControl = 1,
	DSS_Com_ControlRelease = 2,
	DSS_Com_ReleaseDone = 3,

	DSS_HomingSearchSensor = 10,
	DSS_HomingMakeOffset = 11,

	DSS_PositionOperating = 20,

	DSS_ClampingOperating = 30,
	DSS_ClampingConnectControl = 31,

	DSS_ClampingReleaseOperating = 40
} DeviceSubState;

typedef enum __DevType
{
	SC_Type_A2 = 0,
	SC_Type_B0 = 1,
	SC_Type_C1 = 2,
	SC_Type_D = 3,
	SC_Type_E = 4,
	SC_Type_F = 5
} DevType;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile Int16U CONTROL_BootLoaderRequest;
//
extern Int16U CONTROL_Values_1[VALUES_x_SIZE];
extern Int32U CONTROL_Values_1_32[VALUES_x_SIZE];
extern Int16U CONTROL_Values_SubState[VALUES_XLOG_x_SIZE];
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

#endif // __CONTROLLER_H
