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
#define SM_MIN_SPEED				100			// in um/s
#define SM_MAX_SPEED				50000		// in um/s
#define SM_SPEED_CHANGE_STEPS		400			// Acceleration in steps
#define SM_STEPS_RESERVE			10			// Safety area of steps to destination position

// Functions
//
// Main logic ISR call
ISRCALL Timer1_ISR();
// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler);
// Steps Enable
void SM_Enable(Boolean State);
// New position in mm, speed in mm/s
void SM_GoToPositionFromReg(Int16U NewPosition, Int16U MaxSpeed, Int16U LowSpeedPosition, Int16U LowSpeed);
Boolean SM_IsPositioningDone();
// Homing
void SM_Homing();
Boolean SM_IsHomingDone();

#endif // __STEPPER_MOTOR_H
