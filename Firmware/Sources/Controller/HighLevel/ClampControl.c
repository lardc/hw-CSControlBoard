// Header
#include "ClampControl.h"
//
#include "Clamp.h"
#include "Controller.h"
#include "DeviceProfile.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "ZbBoard.h"
#include "ZwTimer.h"
#include "Constraints.h"
//
#include "Global.h"

// Definitions
typedef enum __ClampingState
{
	CS_NONE	= 0,

	// Clamp block
	CS_CLAMP_GOTO_MID_POS,
	CS_CLAMP_DETECT_CLAMPING,
	CS_CLAMP_REGULATOR,
	CS_CLAMP_POSTREGULATOR1,
	CS_CLAMP_POSTREGULATOR2,

	// Release block
	CS_RLS_REGULATOR,
	CS_RLS_CONFGIURE,
	CS_RLS_START_OPERATION,
	CS_RLS_GOTO_START_POS,
	CS_RLS_SERVICE_DELAY

} ClampingState;

// Variables
//
static volatile ClampingState CLAMPCTRL_State = CS_NONE;
static volatile Int32U ClampTimeout, DelayPowerSwitch, DelayTickCounter = 0;
static volatile Boolean CounterOverflow = FALSE;
//
static volatile _iq ForceDesired, ForceActual, ClampDetect, Force2STReleaseMode, Error, Kp, Ki, GearRatio;
static volatile Int16U MaxAllowedError;
static volatile Int32S ControlSignal, PositionActual, ClampMaxIncrements;
static volatile Int16U PositionSpeedLimit, PositionTorqueLimit;
static volatile Int16U ClampMidPosition, ClampTopPosition, ClampSpeedLimit, ClampTorqueLimit;
static volatile Int16U ReleaseTargetPosition;
//
static volatile Boolean ContinuousControl, Use2STReleaseMode, UseClampBreak, UseAirControl;

// Forward functions
//
static void CLAMPCTRL_Apply_Kp(Int16U Kp_N, Int16U Kp_D);
static void CLAMPCTRL_DummyDataLogger();
static void CLAMPCTRL_DataLogger(_iq _ForceActual, _iq _ForceDesired, _iq _ForceError,\
								 Int32S _Control, Int32S _PositionActual);
static Boolean CLAMPCTRL_Cycle();
static Int16U CLAMPCTL_GetMidPosition();
static Int16U CLAMPCTRL_CalcTorqueLimit(Boolean ForceRecalculate);
_iq CLAMPCTRL_GetForceSetpoint();

// Functions
//
void CLAMPCTRL_StartClamping()
{
	ZbAnanlogInput_EnableAcq(TRUE);
	CLAMPCTRL_CacheVariables();

	// Manual brake control
	CLAMP_BrakeManualRelease(TRUE);
	CLAMP_BrakeAutoControl(FALSE);

	CLAMP_SpeedTorqueLimits(PositionSpeedLimit, PositionTorqueLimit);
	CLAMP_GoToPosition_mm(TRUE, ClampMidPosition);
	DELAY_US(1000L * DELAY_OP_COMPLETE);

	CLAMPCTRL_State = CS_CLAMP_GOTO_MID_POS;
}
// ----------------------------------------

Boolean CLAMPCTRL_IsClampingDone()
{
	Boolean Result = FALSE;

	switch(CLAMPCTRL_State)
	{
		case CS_CLAMP_GOTO_MID_POS:
			if(CLAMP_IsTargetReached())
			{
				CLAMP_CompleteOperation(FALSE);
				CLAMP_SpeedTorqueLimits(ClampSpeedLimit, ClampTorqueLimit);
				CLAMP_GoToPosition_mm(FALSE, ClampTopPosition);
				CLAMPCTRL_State = CS_CLAMP_DETECT_CLAMPING;
			}
			break;

		case CS_CLAMP_DETECT_CLAMPING:
			CLAMPCTRL_DummyDataLogger();
			if(ForceActual > ClampDetect)
			{
				CLAMP_QuickStop(TRUE);
				ControlSignal = CLAMP_CurrentIncrements();
				CLAMP_GoToPosition(ControlSignal);
				CLAMP_QuickStop(FALSE);
				DelayTickCounter = 0;
				CLAMPCTRL_State = CS_CLAMP_REGULATOR;
			}
			break;

		case CS_CLAMP_REGULATOR:
			{
				ZbAnanlogInput_EnableAcq(FALSE);
				Result = CLAMPCTRL_Cycle();

				if(++DelayTickCounter > ClampTimeout)
				{
					if(!Result)
					{
						Result = TRUE;
						DataTable[REG_PROBLEM] = PROBLEM_NO_FORCE;
					}
				}

				if(Result)
				{
					if(UseClampBreak)
						CLAMP_BrakeManualRelease(FALSE);
					CLAMPCTRL_Apply_Kp(REG_FORCE_Kp_POST_N, REG_FORCE_Kp_POST_D);

					Result = FALSE;
					DelayTickCounter = 0;
					ZbGPIO_EnablePowerSwitch(TRUE);
					CLAMPCTRL_State = CS_CLAMP_POSTREGULATOR1;
				}
				ZbAnanlogInput_EnableAcq(TRUE);
			}
			break;

		case CS_CLAMP_POSTREGULATOR1:
			{
				ZbAnanlogInput_EnableAcq(FALSE);
				CLAMPCTRL_Cycle();
				ZbAnanlogInput_EnableAcq(TRUE);

				if(++DelayTickCounter > DelayPowerSwitch)
				{
					Result = TRUE;
					CLAMPCTRL_State = CS_CLAMP_POSTREGULATOR2;

					DataTable[REG_PROBLEM] =
							(UseAirControl ? ZbGPIO_PressureOK() : TRUE) ? PROBLEM_NONE : PROBLEM_NO_AIR_PRESSURE;
				}
			}
			break;

		case CS_CLAMP_POSTREGULATOR2:
			{
				ZbAnanlogInput_EnableAcq(FALSE);
				CLAMPCTRL_Cycle();
				ZbAnanlogInput_EnableAcq(TRUE);
			}
			break;
	}

	return Result;
}
// ----------------------------------------

void CLAMPCTRL_XLog(DeviceState State)
{
	Int32S tmp;
	static Boolean EnableSampling = FALSE;

	// Start sampling on clamping start
	if(State == DS_Clamping)
		EnableSampling = TRUE;

	// Disable sampling in ready mode
	if(State == DS_Ready)
		EnableSampling = FALSE;

	if(CONTROL_Values_XLogCounter < VALUES_XLOG_x_SIZE && EnableSampling)
	{
		tmp = _IQint(CLAMP_ReadForce());
		tmp = (tmp < 0) ? 0 : tmp;

		CONTROL_Values_SubState[CONTROL_Values_XLogCounter] = (Int16U)CLAMPCTRL_State;
		CONTROL_Values_Force[CONTROL_Values_XLogCounter] = (Int16U)tmp;
		CONTROL_Values_Error[CONTROL_Values_XLogCounter] = (Int16U)_IQint(Error);
		CONTROL_Values_TorqueLimit[CONTROL_Values_XLogCounter] = CLAMP_GetTorqueLimit();
		CONTROL_Values_XLogCounter++;
	}
}
// ----------------------------------------

_iq CLAMPCTRL_GetForceSetpoint()
{
	return _IQI(100l * DataTable[REG_FORCE_VAL] * DataTable[REG_FORCE_SET_K] / 1000 + 100l * (Int16S)DataTable[REG_FORCE_SET_B]);
}
// ----------------------------------------

void CLAMPCTRL_ClampingUpdateRequest()
{
	ForceDesired = CLAMPCTRL_GetForceSetpoint();
	MaxAllowedError = _IQint(_IQmpy(ForceDesired, _FPtoIQ2(DataTable[REG_CLAMP_ERR_ZONE], 100)));
	ControlSignal = CLAMP_CurrentIncrements();

	ClampTorqueLimit = CLAMPCTRL_CalcTorqueLimit(FALSE);
	CLAMP_SpeedTorqueLimits(ClampSpeedLimit, ClampTorqueLimit);

	CLAMP_BrakeManualRelease(TRUE);
	CLAMP_GoToPosition(ControlSignal);
	DELAY_US(1000L * DELAY_OP_COMPLETE);

	DelayTickCounter = 0;
	CLAMPCTRL_State = CS_CLAMP_REGULATOR;
}
// ----------------------------------------

void CLAMPCTRL_StartClampingRelease(Boolean PositionMode)
{
	// Prepare
	if(CounterOverflow)
	{
		CounterOverflow = FALSE;
		CONTROL_Values_Counter = VALUES_x_SIZE - 1;
	}
	DEVPROFILE_ResetEPReadState();
	CLAMP_BrakeManualRelease(TRUE);

	// Execute clamping release
	if(PositionMode)
	{
		ReleaseTargetPosition = DataTable[REG_CLAMPING_RLS_POS];

		CLAMP_SpeedTorqueLimits(PositionSpeedLimit, MAX(ClampTorqueLimit, PositionTorqueLimit));
		CLAMP_GoToPosition_mm(TRUE, ReleaseTargetPosition);
		CLAMPCTRL_State = CS_RLS_GOTO_START_POS;
	}
	else if(ForceDesired > Force2STReleaseMode && Use2STReleaseMode)
	{
		ForceDesired = Force2STReleaseMode;
		ControlSignal = CLAMP_CurrentIncrements();

		CLAMP_GoToPosition(ControlSignal);
		DELAY_US(1000L * DELAY_OP_COMPLETE);

		DelayTickCounter = 0;
		CLAMPCTRL_State = CS_RLS_REGULATOR;
	}
	else
		CLAMPCTRL_State = CS_RLS_CONFGIURE;
}
// ----------------------------------------

Boolean CLAMPCTRL_IsClampingReleaseDone()
{
	Boolean CycleResult, Result = FALSE;

	switch(CLAMPCTRL_State)
	{
		case CS_RLS_REGULATOR:
			{
				ZbAnanlogInput_EnableAcq(FALSE);
				CycleResult = CLAMPCTRL_Cycle();

				if(++DelayTickCounter > ClampTimeout)
				{
					if(!CycleResult)
					{
						CycleResult = TRUE;
						DataTable[REG_PROBLEM] = PROBLEM_NO_FORCE;
					}
				}

				if(CycleResult)
					CLAMPCTRL_State = CS_RLS_CONFGIURE;
				else
					ZbAnanlogInput_EnableAcq(TRUE);
			}
			break;

		case CS_RLS_CONFGIURE:
			if(CLAMP_IsTargetReached())
			{
				CLAMP_CompleteOperation(FALSE);
				CLAMP_SpeedTorqueLimits(PositionSpeedLimit, MAX(ClampTorqueLimit, PositionTorqueLimit));
				CLAMPCTRL_State = CS_RLS_START_OPERATION;
			}
			break;

		case CS_RLS_START_OPERATION:
			if(CLAMP_IsTargetReached())
			{
				CLAMP_GoToPosition_mm(FALSE, ReleaseTargetPosition);
				CLAMPCTRL_State = CS_RLS_GOTO_START_POS;
			}
			break;

		case CS_RLS_GOTO_START_POS:
			if(CLAMP_IsTargetReached())
			{
				CLAMP_CompleteOperation(FALSE);
				DelayTickCounter = 0;
				CLAMPCTRL_State = CS_RLS_SERVICE_DELAY;
			}
			break;

		case CS_RLS_SERVICE_DELAY:
			{
				if(++DelayTickCounter > SERVICE_DELAY_TICKS)
				{
					// Automatic brake control
					CLAMP_BrakeAutoControl(TRUE);
					Result = TRUE;
					CLAMPCTRL_State = CS_NONE;
				}
			}
			break;
	}

	return Result;
}
// ----------------------------------------

Int16U CLAMPCTL_GetMidPosition()
{
	Int16S mid;
	mid = (Int16S)DataTable[REG_ALLOWED_MOVE] - (DataTable[REG_CLAMPING_DEV_OFFSET] + DataTable[REG_DEV_HEIGHT]);

	return (DataTable[REG_DEV_HEIGHT] == 0 || mid < 0) ? DataTable[REG_CLAMPING_RLS_POS] : mid;
}
// ----------------------------------------

void CLAMPCTRL_CacheVariables()
{
	CLAMPCTRL_Apply_Kp(REG_FORCE_Kp_N, REG_FORCE_Kp_D);

	PositionSpeedLimit = DataTable[REG_POSITION_SPEED_LIMIT];
	PositionTorqueLimit = DataTable[REG_POSITION_TORQUE_LIMIT];

	ClampTimeout = (((Int32U)DataTable[REG_CLAMP_TIMEOUT]) * CS_MONITORING_FREQ) / 1000;
	DelayPowerSwitch = (((Int32U)DataTable[REG_POWER_SW_DELAY]) * CS_MONITORING_FREQ) / 1000;

	ClampMidPosition = CLAMPCTL_GetMidPosition();
	ClampTopPosition = DataTable[REG_BALL_SCREW_STROKE];
	ClampSpeedLimit = DataTable[REG_CLAMP_SPEED_LIMIT];
	ClampTorqueLimit = CLAMPCTRL_CalcTorqueLimit(TRUE);

	GearRatio = _FPtoIQ2(DataTable[REG_GEAR_RATIO_K_N], DataTable[REG_GEAR_RATIO_K_D]);
	ForceDesired = CLAMPCTRL_GetForceSetpoint();

	Force2STReleaseMode = _IQI(100l * DataTable[REG_2ST_FORCE_LIM]);
	Use2STReleaseMode = DataTable[REG_USE_2ST_CLAMP] ? TRUE : FALSE;
	ClampDetect = _IQmpy(ForceDesired, _IQ(0.3f));
	if(ClampDetect < CLAMP_DETECT_LIM)
		ClampDetect = CLAMP_DETECT_LIM;

	ReleaseTargetPosition = DataTable[REG_CLAMPING_RLS_POS];
	ClampMaxIncrements = CLAMP_PositionToTicks(ClampTopPosition);
	UseClampBreak = DataTable[REG_USE_CLAMP_BREAK] ? TRUE : FALSE;
	UseAirControl = DataTable[REG_USE_AIR_CONTROL] ? TRUE : FALSE;
	ContinuousControl = DataTable[REG_USE_CLAMP_BREAK] ? FALSE : DataTable[REG_CONTINUOUS_CTRL];
	MaxAllowedError = _IQint(_IQmpy(ForceDesired, _FPtoIQ2(DataTable[REG_CLAMP_ERR_ZONE], 100)));
}
// ----------------------------------------

static Int16U CLAMPCTRL_CalcTorqueLimit(Boolean ForceRecalculate)
{
	Int16U currClampTorqueLimit, newClampTorqueLimit;

	currClampTorqueLimit = CLAMP_GetTorqueLimit();
	newClampTorqueLimit =
			_IQint(
					_IQsat(_IQmpy(_IQI(((Int32U)DataTable[REG_CLAMP_TORQUE_LIMIT]) * DataTable[REG_FORCE_VAL] / FORCE_VAL_MAX), _IQ(1.5f)), _IQI(DataTable[REG_CLAMP_TORQUE_LIMIT]), CLAMP_LOWEST_TORQUE));

	if(ForceRecalculate)
		return newClampTorqueLimit;
	else
		return (newClampTorqueLimit > currClampTorqueLimit) ? newClampTorqueLimit : currClampTorqueLimit;
}
// ----------------------------------------

static void CLAMPCTRL_Apply_Kp(Int16U Kp_N_index, Int16U Kp_D_index)
{
	Kp = _FPtoIQ2(DataTable[Kp_N_index], DataTable[Kp_D_index]);
}
// ----------------------------------------

static Boolean CLAMPCTRL_Cycle()
{
	Boolean ClampingDone = FALSE;

	ForceActual = CLAMP_ReadForce();
	Error = ForceDesired - ForceActual;
	PositionActual = CLAMP_CurrentPosition() / 10000;

	if(CLAMPCTRL_State == CS_CLAMP_REGULATOR || CLAMPCTRL_State == CS_RLS_REGULATOR
			|| (ContinuousControl
					&& (CLAMPCTRL_State == CS_CLAMP_POSTREGULATOR1 || CLAMPCTRL_State == CS_CLAMP_POSTREGULATOR2)))
	{
		if(CLAMP_IsPositionReached())
		{
			if(ABS(_IQint(Error)) > MaxAllowedError)
			{
				CLAMP_CompleteOperation(FALSE);
				// Delay for operation complete command
				DELAY_US(1000L * DELAY_OP_COMPLETE);
				ControlSignal += _IQint(_IQmpy(GearRatio, NEUTON_TO_INCREMENT)) * _IQint(_IQmpy(Error, Kp));

				if(ControlSignal > ClampMaxIncrements)
					ControlSignal = ClampMaxIncrements;
				CLAMP_GoToPosition(ControlSignal);
			}
			else
				ClampingDone = TRUE;
		}
	}
	CLAMPCTRL_DataLogger(ForceActual, ForceDesired, Error, ControlSignal, PositionActual);

	return ClampingDone;
}
// ----------------------------------------

static void CLAMPCTRL_DummyDataLogger()
{
	ForceActual = CLAMP_ReadForce();
	PositionActual = CLAMP_CurrentPosition() / 10000;
	CLAMPCTRL_DataLogger(ForceActual, 0, 0, 0, PositionActual);
}
// ----------------------------------------

static void CLAMPCTRL_DataLogger(_iq _ForceActual, _iq _ForceDesired, _iq _ForceError, Int32S _Control,
		Int32S _PositionActual)
{
	CONTROL_Values_1[CONTROL_Values_Counter] = (_ForceActual > 0) ? _IQint(_IQdiv(_ForceActual, _IQ(10))) : 0;
	CONTROL_Values_2[CONTROL_Values_Counter] = _IQint(_IQdiv(_ForceDesired, _IQ(10)));
	CONTROL_Values_3[CONTROL_Values_Counter] = _IQint(_IQdiv(_ForceError, _IQ(10)));
	//
	CONTROL_Values_1_32[CONTROL_Values_Counter] = _Control;
	CONTROL_Values_2_32[CONTROL_Values_Counter] = _PositionActual;

	CONTROL_Values_Counter++;

	if(CONTROL_Values_Counter >= VALUES_x_SIZE)
	{
		CounterOverflow = TRUE;
		CONTROL_Values_Counter = 0;
	}
}
// ----------------------------------------
