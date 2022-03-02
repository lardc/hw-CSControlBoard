// ----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"

// Types
typedef void (*xTimerAlterHandler)();

// Variables
static xTimerAlterHandler AlterHandler = NULL;

static Int32S SM_GlobalStepsCounter = 0, SM_DestSteps = 0;
static Int16U SM_LowSpeedSteps, SM_CyclesToToggle, SM_LowSpeedCycles, SM_MinCycles, SM_MaxCycles;
static Boolean SM_IsCurrentDirUp, SM_HomingFlag;

// Forward functions
void SM_LogicHandler();
Int16U SM_SpeedToCyclesRaw(Int16U Value);
Int16U SM_SpeedToCycles(Int16U NewSpeed);
Int32U SM_PosToSteps(Int32U NewPos);
void SM_UpDirection(Boolean State);
void SM_GoToPosition(Int32U NewPosition, Int16U MaxSpeed, Int32U LowSpeedPosition, Int16U LowSpeed);

// Functions
//
// Timer 1 ISR
ISRCALL Timer1_ISR()
{
	if(AlterHandler)
	{
		xTimerAlterHandler AlterHandlerCopy = AlterHandler;
		AlterHandlerCopy();
	}
	else
		SM_LogicHandler();

	// no PIE
	TIMER1_ISR_DONE;
}
// -----------------------------------------

// Connect alter handler for timer processing
void SM_ConnectAlterHandler(void *Handler)
{
	AlterHandler = (xTimerAlterHandler)Handler;
}
// ----------------------------------------

// Main logic handler
void SM_LogicHandler()
{
	static Boolean TickHigh = FALSE;
	static Int16U SM_CycleCounter = 0;

	if(!SM_IsPositioningDone() || SM_HomingFlag)
	{
		// Генератор шагов
		if(++SM_CycleCounter >= SM_CyclesToToggle)
		{
			SM_CycleCounter = 0;
			ZbGPIO_SwitchStep(TickHigh = !TickHigh);

			// Счёт для перемещения по нарастающему фронту тиков
			if(TickHigh)
			{
				if(SM_HomingFlag)
				{
					// Условие завершения хоуминга
					if(ZbGPIO_HomeSensorActuate())
					{
						SM_HomingFlag = FALSE;
						SM_DestSteps = SM_GlobalStepsCounter = 0;
					}
				}
				else
				{
					// Проверка условия позиционирования
					SM_GlobalStepsCounter += (SM_IsCurrentDirUp) ? 1 : -1;
					Int32U StepsToPos = abs(SM_DestSteps - SM_GlobalStepsCounter);

					// 1-2. acceleration to Vmax or running at Vmax
					if(StepsToPos > SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
					{
						if(SM_CyclesToToggle > SM_MinCycles)
							--SM_CyclesToToggle;
					}

					// 3. acceleration or deceleration to Vend
					else if(StepsToPos > (Int32U)SM_SPEED_CHANGE_STEPS * SM_MinCycles / SM_LowSpeedCycles + SM_LowSpeedSteps + SM_STEPS_RESERVE)
					{
						if(SM_CyclesToToggle < SM_LowSpeedCycles)
							++SM_CyclesToToggle;
						else if(SM_CyclesToToggle > SM_LowSpeedCycles)
							--SM_CyclesToToggle;
					}

					// 4. stable Vend or acceleration to Vend
					else if(StepsToPos > (Int32U)SM_SPEED_CHANGE_STEPS * SM_MinCycles / SM_LowSpeedCycles + SM_STEPS_RESERVE)
					{
						if(SM_CyclesToToggle > SM_LowSpeedCycles)
							--SM_CyclesToToggle;
					}

					// 5. deceleration to Vmin
					else
					{
						if(SM_CyclesToToggle < SM_MaxCycles)
							++SM_CyclesToToggle;
					}
				}
			}
		}
	}
}
// -----------------------------------------

// Up or down direction
void SM_UpDirection(Boolean State)
{
	ZbGPIO_SwitchDir(SM_IsCurrentDirUp = State);
}
// ----------------------------------------

// Steps Enable
void SM_Enable(Boolean State)
{
	ZbGPIO_SwitchEnable(!State);
}
// ----------------------------------------

// New position in um, speed in um/s
void SM_GoToPosition(Int32U NewPosition, Int16U MaxSpeed, Int32U LowSpeedPosition, Int16U LowSpeed)
{
	SM_LowSpeedSteps = SM_PosToSteps(NewPosition - LowSpeedPosition);
	SM_DestSteps = SM_PosToSteps(NewPosition);

	SM_UpDirection(SM_DestSteps > SM_GlobalStepsCounter);

	SM_MinCycles = SM_SpeedToCycles(MaxSpeed);
	SM_LowSpeedCycles = SM_SpeedToCycles(LowSpeed);
	SM_CyclesToToggle = SM_MaxCycles = SM_SpeedToCycles(SM_MIN_SPEED);
}
// ----------------------------------------

// New position in mm, speed in um/s
void SM_GoToPositionFromReg(Int16U NewPositionReg, Int16U MaxSpeedReg, Int16U LowSpeedPositionReg,
		Int16U LowSpeedReg)
{
	SM_GoToPosition((Int32U)NewPositionReg * 1000, MaxSpeedReg,
			(Int32U)LowSpeedPositionReg * 1000, (Int16U)LowSpeedReg * 1000);
}
// ----------------------------------------

// Homing
void SM_Homing()
{
	SM_HomingFlag = TRUE;
	SM_CyclesToToggle = 10;
}
// ----------------------------------------

// Is homing done?
Boolean SM_IsHomingDone()
{
	return !SM_HomingFlag && SM_IsPositioningDone();
}
// ----------------------------------------

Boolean SM_IsPositioningDone()
{
	return SM_DestSteps == SM_GlobalStepsCounter;
}
// ----------------------------------------

// Position in um to steps converter
Int32U SM_PosToSteps(Int32U NewPos)
{
	return NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND;
}
// ----------------------------------------

// Speed in um/s to cycles to toggle converter
Int16U SM_SpeedToCycles(Int16U NewSpeed)
{
	if(NewSpeed < SM_MIN_SPEED)
		return SM_SpeedToCyclesRaw(SM_MIN_SPEED);
	else if(NewSpeed < SM_MAX_SPEED)
		return SM_SpeedToCyclesRaw(NewSpeed);
	else
		return SM_SpeedToCyclesRaw(SM_MAX_SPEED);
}
// ----------------------------------------

// Speed in um/s to cycles to toggle raw converter
Int16U SM_SpeedToCyclesRaw(Int16U Value)
{
	Int32U res = 1000000ul / TIMER1_PERIOD * SM_MOVING_RER_ROUND / SM_FULL_ROUND_STEPS / Value;
	return (res == 0) ? 1 : res;
}
// ----------------------------------------
