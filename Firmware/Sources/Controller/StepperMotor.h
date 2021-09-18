// -----------------------------------------
// SM driver module
// ----------------------------------------

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

// Include
#include "Global.h"
#include "ZbBoard.h"
#include "SysConfig.h"

// Definitions
//
#define SM_DEFAULT_CYCLES_TO_TOGGLE	1000
#define SM_MIN_SPEED				100			// in um/s
#define SM_MAX_SPEED				50000		// in um/s
#define SM_HOMING_SPEED				5000		// in um/s
#define SM_HOMING_RESERVE			2000		// Safety area in um from homing detection to origin
#define SM_SPEED_CHANGE_STEPS		400			// Acceleration in steps
#define SM_STEPS_RESERVE			10			// Safety area of steps to destination position
#define SM_S_TO_US					1000000		// Seconds to us convertation coefficient

// CPU Timer 2 ISR
ISRCALL Timer1_ISR();

// Functions
//
// Next step generate check
Boolean SM_IsGenerateNextStep();
// Update steps period in us
Boolean SM_UpdPeriod(Int16U NewPeriod);
// Update steps in cycles to toggle
Boolean SM_UpdCyclesToToggle(Int16U NewCyclesToTogle);
// Period to cycles to toggle converter
Int16U SM_PeriodToCycles(Int16U NewPeriod);
// Start Steps
void SM_SetStartSteps();
// Stop Steps
void SM_SetStopSteps();
// Up Direction
void SM_UpDir();
// Down Direction
void SM_DownDir();
// Steps Enable
void SM_Enable(Boolean State);
// New position in um, speed in um/s
Boolean SM_GoToPosition(Int32U NewPosition, Int16U MaxSpeed, Int32U LowSpeedPosition, Int16U LowSpeed);
// New position in mm, speed in mm/s
Boolean SM_GoToPositionFromReg(Int16U NewPosition, Int16U MaxSpeed, Int16U LowSpeedPosition, Int16U LowSpeed);
// Homing
void SM_Homing();
// Set new origin
void SM_SetOrigin();
// Position in um to steps converter
Int32U SM_PosToSteps(Int32U NewPos);
// Speed in um/s to cycles to toggle converter
Int16U SM_SpeedToCycles(Int16U NewSpeed);
// Get current position in um
Int32U SM_GetPosition();
Boolean SM_IsHomingDone();
Boolean SM_IsSlidingDone();

#endif // __STEPPER_MOTOR_H

