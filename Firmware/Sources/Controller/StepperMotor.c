// -----------------------------------------
// SM driver module
// ----------------------------------------

// Header
#include "StepperMotor.h"

// Variables
//
static volatile Int16U SM_CycleCounter = 0;
static volatile Int16U SM_GlobalStepsCounter = 0;							// in steps*2
static volatile Int16U SM_PrevGlobalStepsCounter = 0;
static volatile Int16U SM_CyclesToToggle = SM_DEFAULT_CYCLES_TO_TOGGLE;

Int16U SM_StartSteps = 0;
Int16U SM_DestSteps = 0;
Int16U SM_LowSpeedSteps = 0;

Int16U SM_StartCycles = 0;
Int16U SM_DestCycles = 0;
Int16U SM_LowSpeedCycles = 0;
Int16U SM_MaxCycles = 0;
Int16U SM_MinCycles = 0;

static Int16U SM_SpeedChangeDiscrete = 0;

Boolean SM_IsCurrentDirUp = TRUE;
Boolean SM_HomingFlag = FALSE;
Boolean SM_OriginFlag = FALSE;
Boolean SM_ChangePositionFlag = FALSE;
Boolean SM_ChangePositionInit = FALSE;

// Timer 1 ISR
ISRCALL Timer1_ISR(void)
{
	// Steps generator
	if(SM_CycleCounter >= SM_CyclesToToggle)
	{
		ZbGPIO_ToggleStep();
		SM_CycleCounter = 0;
		if(SM_IsCurrentDirUp)
			SM_GlobalStepsCounter++;
		else
			SM_GlobalStepsCounter--;
	}
	else
		++SM_CycleCounter;

	// Changing position processor
	if(SM_ChangePositionFlag)
	{
		Int16U StepsToPos = abs(SM_DestSteps - SM_GlobalStepsCounter);
		// First init
		if(SM_ChangePositionInit)
		{
			SM_SpeedChangeDiscrete = (Int16U)(SM_SPEED_CHANGE_STEPS / (SM_MaxCycles - SM_MinCycles));
			SM_UpdCyclesToToggle(SM_MaxCycles);
			SM_ChangePositionInit = FALSE;
		}
		else
		{
			// 1. acceleration to Vmax
			if(StepsToPos > 2 * SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
			{
				if(SM_IsGenerateNextStep())
				{
					SM_PrevGlobalStepsCounter = SM_GlobalStepsCounter;
					if(SM_CyclesToToggle > SM_MinCycles)
						SM_UpdCyclesToToggle(--SM_CyclesToToggle);
				}
			}
			// 2. stable Vmax or acceleration to Vmax
			else if(StepsToPos > SM_SPEED_CHANGE_STEPS + SM_LowSpeedSteps + SM_STEPS_RESERVE)
			{
				if(SM_CyclesToToggle != SM_MinCycles)
				{
					if(SM_IsGenerateNextStep())
					{
						SM_PrevGlobalStepsCounter = SM_GlobalStepsCounter;
						if(SM_CyclesToToggle > SM_MinCycles)
							SM_UpdCyclesToToggle(--SM_CyclesToToggle);
					}
				}
			}
			// 3. acceleration or deceleration to Vend
			else if(StepsToPos
					> SM_SPEED_CHANGE_STEPS * (SM_MinCycles / SM_LowSpeedCycles) + SM_LowSpeedSteps + SM_STEPS_RESERVE)
			{
				if(SM_IsGenerateNextStep())
				{
					SM_PrevGlobalStepsCounter = SM_GlobalStepsCounter;
					if(SM_CyclesToToggle < SM_LowSpeedCycles)
						SM_UpdCyclesToToggle(++SM_CyclesToToggle);
					else if(SM_CyclesToToggle > SM_LowSpeedCycles)
						SM_UpdCyclesToToggle(--SM_CyclesToToggle);
				}
			}
			// 4. stable Vend or acceleration to Vend
			else if(StepsToPos > SM_SPEED_CHANGE_STEPS * (SM_MinCycles / SM_LowSpeedCycles) + SM_STEPS_RESERVE)
			{
				if(SM_CyclesToToggle != SM_LowSpeedCycles)
				{
					if(SM_IsGenerateNextStep())
					{
						SM_PrevGlobalStepsCounter = SM_GlobalStepsCounter;
						if(SM_CyclesToToggle > SM_LowSpeedCycles)
							SM_UpdCyclesToToggle(--SM_CyclesToToggle);
					}
				}
			}
			// 5. deceleration to Vmin
			else if(StepsToPos > SM_STEPS_RESERVE)
			{
				if(SM_IsGenerateNextStep())
				{
					SM_PrevGlobalStepsCounter = SM_GlobalStepsCounter;
					if(SM_CyclesToToggle < SM_MaxCycles)
						SM_UpdCyclesToToggle(++SM_CyclesToToggle);
				}
			}
			if(StepsToPos == 0)
			{
				if(SM_HomingFlag)
				{
					if(ZbGPIO_HomeSensorActuate())
					{
						SM_SetStopSteps();
						SM_ChangePositionFlag = FALSE;
						SM_GlobalStepsCounter = 0;
						SM_HomingFlag = FALSE;
						SM_SetOrigin();
					}
				}
				else if(SM_OriginFlag)
				{
					SM_SetStopSteps();
					SM_ChangePositionFlag = FALSE;
					SM_GlobalStepsCounter = 0;
					SM_OriginFlag = FALSE;
				}
				else
				{
					SM_SetStopSteps();
					SM_ChangePositionFlag = FALSE;
				}
			}
		}
	}
	// no PIE
	TIMER1_ISR_DONE;
}
// -----------------------------------------

// Functions
//
// Next step generate check
Boolean SM_IsGenerateNextStep()
{
	return (((abs(SM_GlobalStepsCounter - SM_StartSteps) % SM_SpeedChangeDiscrete) == 0) && (SM_PrevGlobalStepsCounter != SM_GlobalStepsCounter));
}
// ----------------------------------------

// Update steps period in us
Boolean SM_UpdPeriod(Int16U NewPeriod)
{
	Int16U NewCyclesToTogle = SM_PeriodToCycles(NewPeriod);
	return SM_UpdCyclesToToggle(NewCyclesToTogle);
}
// ----------------------------------------

// Update steps in cycles to toggle
Boolean SM_UpdCyclesToToggle(Int16U NewCyclesToTogle)
{
	if(NewCyclesToTogle <= 1)
		return FALSE;
	else
	{
		SM_CyclesToToggle = NewCyclesToTogle;
		SM_CycleCounter = 0;
		return TRUE;
	}
}
// ----------------------------------------

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
}
// ----------------------------------------

// Stop Steps
void SM_SetStopSteps()
{
	ZwTimer_EnableInterruptsT1(FALSE);
	ZbGPIO_SwitchStep(TRUE);
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
	ZbGPIO_SwitchEnable(State);
}
// ----------------------------------------

// New position in um, speed in um/s
Boolean SM_GoToPosition(Int32U NewPosition, Int16U MaxSpeed, Int32U LowSpeedPosition, Int16U LowSpeed)
{
	// validation
	if((NewPosition <= SM_MAX_POSITION) && (LowSpeedPosition <= SM_MAX_POSITION))
	{
		SM_StartSteps = SM_GlobalStepsCounter;
		SM_LowSpeedSteps = SM_PosToSteps(abs(NewPosition - LowSpeedPosition));
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

		SM_ChangePositionFlag = TRUE;
		SM_ChangePositionInit = TRUE;

		SM_SetStartSteps();
		return TRUE;
	}
	else
		return FALSE;
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
	return ! SM_ChangePositionFlag;
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
	if(NewSpeed <= SM_MIN_SPEED)
	{
		return (Int16U)(SM_S_TO_US * SM_MOVING_RER_ROUND / (2 * SM_MIN_SPEED * SM_FULL_ROUND_STEPS * TIMER1_PERIOD));
	}
	else if(NewSpeed < SM_MAX_SPEED)
	{
		return (Int16U)(SM_S_TO_US * SM_MOVING_RER_ROUND / (2 * NewSpeed * SM_FULL_ROUND_STEPS * TIMER1_PERIOD));
	}
	else
		return (Int16U)(SM_S_TO_US * SM_MOVING_RER_ROUND / (2 * SM_MAX_SPEED * SM_FULL_ROUND_STEPS * TIMER1_PERIOD));
}
// ----------------------------------------

// Get current position in um
Int32U SM_GetPosition()
{
	return (Int32U)(SM_MOVING_RER_ROUND * SM_GlobalStepsCounter / (2 * SM_FULL_ROUND_STEPS));
}
// ----------------------------------------

// No more
