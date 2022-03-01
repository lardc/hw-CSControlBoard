// ----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"

// Definitions
#define SPEED_TO_CYCL_RAW(a)	(SM_S_TO_US * SM_MOVING_RER_ROUND / (2 * (a) * SM_FULL_ROUND_STEPS * TIMER1_PERIOD))

// Types
typedef void (*xTimerAlterHandler)();

// Variables
static xTimerAlterHandler AlterHandler = NULL;

Int32S SM_GlobalStepsCounter = 0;
volatile Int16U SM_CyclesToToggle = SM_DEFAULT_CYCLES_TO_TOGGLE;

Int16U SM_StartSteps = 0;
Int16U SM_DestSteps = 0;
Int16U SM_LowSpeedSteps = 0;

Int16U SM_StartCycles = 0;
Int16U SM_DestCycles = 0;
Int16U SM_LowSpeedCycles = 0;
Int16U SM_MaxCycles = 0;
Int16U SM_MinCycles = 0;
Int32U StepsToPos = 1;

static Int16U SM_SpeedChangeDiscrete = 0;

Boolean SM_IsCurrentDirUp = TRUE;
Boolean SM_HomingFlag = FALSE;
Boolean SM_OriginFlag = FALSE;

// Forward functions
//
void SM_LogicHandler();

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

	if(!SM_IsSlidingDone())
	{
		// √енератор шагов
		if(++SM_CycleCounter >= SM_CyclesToToggle)
		{
			SM_CycleCounter = 0;
			ZbGPIO_SwitchStep(TickHigh = !TickHigh);

			// —чЄт по нарастающему фронту тиков
			if(TickHigh)
			{
				SM_GlobalStepsCounter += (SM_IsCurrentDirUp) ? 1 : -1;
				Int32U StepsToPos = abs(SM_DestSteps - SM_GlobalStepsCounter);

				// 1. acceleration to Vmax
				if(StepsToPos > 2 * SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
				{
					if(SM_CyclesToToggle > SM_MinCycles)
						--SM_CyclesToToggle;
				}

				// 2. stable Vmax or acceleration to Vmax
				else if(StepsToPos > SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
				{
					if(SM_CyclesToToggle > SM_MinCycles)
						--SM_CyclesToToggle;
				}

				// 3. acceleration or deceleration to Vend
				else if(StepsToPos > SM_SPEED_CHANGE_STEPS * (SM_MinCycles / SM_LowSpeedCycles) + SM_LowSpeedSteps + SM_STEPS_RESERVE)
				{
					if(SM_CyclesToToggle < SM_LowSpeedCycles)
						++SM_CyclesToToggle;
					else if(SM_CyclesToToggle > SM_LowSpeedCycles)
						--SM_CyclesToToggle;
				}

				// 4. stable Vend or acceleration to Vend
				else if(StepsToPos > SM_SPEED_CHANGE_STEPS * (SM_MinCycles / SM_LowSpeedCycles) + SM_STEPS_RESERVE)
				{
					if(SM_CyclesToToggle > SM_LowSpeedCycles)
						--SM_CyclesToToggle;
				}

				// 5. deceleration to Vmin
				else if(StepsToPos > SM_STEPS_RESERVE)
				{
					if(SM_CyclesToToggle < SM_MaxCycles)
						++SM_CyclesToToggle;
				}

				if(StepsToPos == 0)
				{
					if(SM_HomingFlag)
					{
						//SM_DestSteps = SM_GlobalStepsCounter;
						if(ZbGPIO_HomeSensorActuate())
						{
							SM_SetStopSteps();
							SM_GlobalStepsCounter = 0;
							SM_HomingFlag = FALSE;
							SM_SetOrigin();
						}
					}
					else if(SM_OriginFlag)
					{
						SM_GlobalStepsCounter = 0;
						SM_DestSteps = 0;
						SM_OriginFlag = FALSE;
						SM_SetStopSteps();
					}
					else
						SM_SetStopSteps();
				}
			}
		}
	}
}
// -----------------------------------------

void SM_PreparePosChange()
{
	StepsToPos = abs(SM_DestSteps - SM_GlobalStepsCounter);
	SM_SpeedChangeDiscrete = (Int16U)(SM_SPEED_CHANGE_STEPS / (SM_MaxCycles - SM_MinCycles));
	SM_CyclesToToggle = 10;
}
// -----------------------------------------

// Period to cycles to toggle converter
Int16U SM_PeriodToCycles(Int16U NewPeriod)
{
	return (int)NewPeriod / (2 * TIMER1_PERIOD);
}
// ----------------------------------------

// Start steps
void SM_SetStartSteps()
{
	ZwTimer_EnableInterruptsT1(TRUE);
	ZwTimer_StartT1();
}
// ----------------------------------------

// Stop Steps
void SM_SetStopSteps()
{
	ZwTimer_EnableInterruptsT1(FALSE);
	ZwTimer_StopT1();
	ZbGPIO_SwitchStep(TRUE);
	SM_Enable(FALSE);
}
// ----------------------------------------

// Up Direction
void SM_UpDir()
{
	ZbGPIO_SwitchDir(TRUE);
	SM_IsCurrentDirUp = TRUE;
}
// ----------------------------------------

// Down Direction
void SM_DownDir()
{
	ZbGPIO_SwitchDir(FALSE);
	SM_IsCurrentDirUp = FALSE;
}
// ----------------------------------------

// Steps Enable
void SM_Enable(Boolean State)
{
	ZbGPIO_SwitchEnable(!State);
}
// ----------------------------------------

// New position in um, speed in um/s
Boolean SM_GoToPosition(Int32U NewPosition, Int16U MaxSpeed, Int32U LowSpeedPosition, Int16U LowSpeed)
{
	// validation
	if((NewPosition <= SM_MAX_POSITION) && (LowSpeedPosition <= SM_MAX_POSITION))
	{
		SM_Enable(TRUE);
		SM_StartSteps = SM_GlobalStepsCounter;
		SM_LowSpeedSteps = SM_PosToSteps(NewPosition - LowSpeedPosition);
		SM_DestSteps = SM_PosToSteps(NewPosition);

		if(SM_DestSteps > SM_StartSteps)
		{
			SM_UpDir();
		}
		else
			SM_DownDir();

		SM_MinCycles = SM_SpeedToCycles(MaxSpeed);
		SM_LowSpeedCycles = SM_SpeedToCycles(LowSpeed);
		SM_MaxCycles = SM_SpeedToCycles(SM_MIN_SPEED);

		SM_PreparePosChange();
		SM_SetStartSteps();
		return TRUE;
	}
	else
		return FALSE;
}

// New position in mm, speed in um/s
Boolean SM_GoToPositionFromReg(Int16U NewPositionReg, Int16U MaxSpeedReg, Int16U LowSpeedPositionReg,
		Int16U LowSpeedReg)
{
	return SM_GoToPosition((Int32U)NewPositionReg * 1000, (Int16U)MaxSpeedReg * 1000,
			(Int32U)LowSpeedPositionReg * 1000, (Int16U)LowSpeedReg * 1000);
}
// ----------------------------------------

// Homing
void SM_Homing()
{
	SM_HomingFlag = TRUE;
	SM_GoToPosition(0, SM_HOMING_SPEED, 0, 0);
}
// ----------------------------------------

// Set new origin
void SM_SetOrigin()
{
	SM_OriginFlag = TRUE;
	SM_GoToPosition(SM_HOMING_RESERVE, SM_HOMING_SPEED, 0, 0);
}
// ----------------------------------------

// Is homing done?
Boolean SM_IsHomingDone()
{
	return !(SM_HomingFlag || SM_OriginFlag);
}
// ----------------------------------------

Boolean SM_IsSlidingDone()
{
	return (SM_DestSteps == SM_GlobalStepsCounter);
}
// ----------------------------------------

// Position in um to steps converter
Int32U SM_PosToSteps(Int32U NewPos)
{
	return (Int16U)(NewPos * SM_FULL_ROUND_STEPS / SM_MOVING_RER_ROUND);
}
// ----------------------------------------

// Speed in um/s to cycles to toggle converter
Int16U SM_SpeedToCycles(Int16U NewSpeed)
{
	if(NewSpeed < SM_MIN_SPEED)
		return SPEED_TO_CYCL_RAW(SM_MIN_SPEED);
	else if(NewSpeed < SM_MAX_SPEED)
		return SPEED_TO_CYCL_RAW(NewSpeed);
	else
		return SPEED_TO_CYCL_RAW(SM_MAX_SPEED);
}
// ----------------------------------------

// Get current position in um
Int32U SM_GetPosition()
{
	return (Int32U)(SM_MOVING_RER_ROUND * SM_GlobalStepsCounter / (2 * SM_FULL_ROUND_STEPS));
}
// ----------------------------------------
