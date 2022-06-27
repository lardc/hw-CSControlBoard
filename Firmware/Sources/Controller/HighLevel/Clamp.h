#ifndef __CLAMP_H
#define __CLAMP_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"
#include "IQmathUtils.h"
#include "ClampSettings.h"

// Functions
//
void CLAMP_CompleteOperation(Boolean InhibitController);
void CLAMP_QuickStop(Boolean Enable);
//
void CLAMP_BrakeAutoControl(Boolean Flag);
void CLAMP_BrakeManualRelease(Boolean Flag);
void CLAMP_EnableController(Boolean Flag);
//
void CLAMP_HomingStart();
Boolean CLAMP_IsHomingDone();
Boolean CLAMP_IsHomingPosAvailable();
//
void CLAMP_SpeedTorqueLimits(Int16U Speed, Int16U Torque);
Int16U CLAMP_GetTorqueLimit();
void CLAMP_HomingRefMLimit(Int16U Torque);
void CLAMP_GoToPosition(Int32S Position);
void CLAMP_GoToPosition_mm(Boolean EnableController, Int32S Position);
Boolean CLAMP_IsTargetReached();
Boolean CLAMP_IsPositionReached();
Int32S CLAMP_CurrentIncrements();
Int32S CLAMP_CurrentPosition();
//
void CLAMP_WriteRegister(Int16U Index, Int16U Subcode, Int32U Value);
Int32U CLAMP_ReadRegister(Int16U Index, Int16U Subcode);
Int32U CLAMP_ReadError();
_iq CLAMP_ReadForce();
void CLAMP_ReadADCOffset();
Boolean CLAMP_IsStopLimitReached();
Boolean CLAMP_IsQSPActive();

#endif // __CLAMP_H
