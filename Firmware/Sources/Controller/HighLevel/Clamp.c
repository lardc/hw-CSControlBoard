// Header
#include "Clamp.h"
//
#include "ZbAnalogInput.h"
#include "DeviceProfile.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
//
#include "SysConfig.h"
#include "Global.h"

// Variables
static Int16U PDOMessage[8];
static Int32S PreviousPosition;
static Int16U ActualTorqueLimit = 0;

// Functions
//
void CLAMP_CompleteOperation(Boolean InhibitController)
{
	// Send PDO
	MemZero16(PDOMessage, 8);
	PDOMessage[0] = 0x33;
	CANopen_PdoWr(&DEVICE_CANopen_Interface, PDOMessage);

	// Enable controller
	if (InhibitController) CLAMP_EnableController(FALSE);
}
// ----------------------------------------

void CLAMP_BrakeAutoControl(Boolean Flag)
{
	/*
	 * ”правление тормозом игнорируетс€
	if (Flag) CLAMP_BrakeManualRelease(FALSE);
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x55eb, 0, Flag ? 12 : 11);
	 */
}
// ----------------------------------------

void CLAMP_BrakeManualRelease(Boolean Flag)
{
	/*
	 * ”правление тормозом игнорируетс€
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5e29, 1, Flag ? 1 : 0);
	 */
}
// ----------------------------------------

void CLAMP_EnableController(Boolean Flag)
{
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5ffd, 16, (Flag) ? 1 : 0);
}
// ----------------------------------------

Int32U CLAMP_ReadError()
{
	return CLAMP_ReadRegister(0x5f5f, 1);
}
// ----------------------------------------

Int32U CLAMP_ReadRegister(Int16U Index, Int16U Subcode)
{
	Int32U Value;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, Index, Subcode, &Value);
	return Value;
}
// ----------------------------------------

void CLAMP_WriteRegister(Int16U Index, Int16U Subcode, Int32U Value)
{
	CANopen_SdoWr(&DEVICE_CANopen_Interface, Index, Subcode, Value);
}
// ----------------------------------------

void CLAMP_HomingStart()
{
	Int32U Data;

#if (PATCH_LENZE_DI6_INVERT == TRUE)
	// Configure DI6 input inversion
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5f8d, 0, 0x20);
#endif

	// Read device state
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5f76, 0, &Data);

	// If device state - ready to switch on
	if (Data == 3)
	{
		// Node start
		CANopen_NMT_NodeStart(&DEVICE_CANopen_Interface, CANOPEN_SLAVE_NODE_ID);

		// Touch to enable normal operation
		CLAMP_EnableController(FALSE);
	}

	// Automatic brake control
	CLAMP_BrakeAutoControl(TRUE);

	// Default values for speed and torque values
	CLAMP_HomingRefMLimit(DataTable[REG_HOMING_TORQUE_LIMIT]);
	CLAMP_SpeedTorqueLimits(DataTable[REG_POSITION_SPEED_LIMIT], DataTable[REG_HOMING_TORQUE_LIMIT]);

	// Configure homing speed
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5b37, 1, DataTable[REG_HOMING_REF_SPEED] * (1ul << 15));
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5b37, 2, DataTable[REG_HOMING_SEARCH_SPEED] * (1ul << 15));

	// Configure homing offset
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5b34, 1, DataTable[REG_HOMING_OFFSET] * (1ul << 15));

	// Enable controller
	CLAMP_EnableController(TRUE);

	// Send PDO
	MemZero16(PDOMessage, 8);
	PDOMessage[0] = 0x11;
	PDOMessage[1] = 0x01;
	CANopen_PdoWr(&DEVICE_CANopen_Interface, PDOMessage);
}
// ----------------------------------------

Boolean CLAMP_IsHomingDone()
{
	Int32U Sdo = 0;

	// Read homing status
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5b26, 0, &Sdo);
	if ((Sdo & BIT12) == 0)
		return FALSE;
	else
		return TRUE;
}
// ----------------------------------------

Boolean CLAMP_IsHomingPosAvailable()
{
	Int32U Sdo = 0;

	// Read homing status
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5b26, 0, &Sdo);
	if ((Sdo & BIT13) == 0)
		return FALSE;
	else
		return TRUE;
}
// ----------------------------------------

void CLAMP_QuickStop(Boolean Enable)
{
	// Speed limits
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5ffd, 17, Enable ? 1 : 0);
}
// ----------------------------------------

void CLAMP_SpeedTorqueLimits(Int16U Speed, Int16U Torque)
{
	ActualTorqueLimit = Torque;

	// Speed limits
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5c72, 1, Speed * 100);
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5c72, 2, Speed * 100);

	// Torque limits
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5e27, 3, Torque * 100);
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5e27, 4, Torque * 100);
}
// ----------------------------------------

Int16U CLAMP_GetTorqueLimit()
{
	return ActualTorqueLimit;
}
// ----------------------------------------

void CLAMP_HomingRefMLimit(Int16U Torque)
{
	CANopen_SdoWr(&DEVICE_CANopen_Interface, 0x5b39, 0, Torque * 100);
}
// ----------------------------------------

void CLAMP_GoToPosition(Int32S Position)
{
	// Exclude zero input
	Position = (Position) ? Position : 1;
	Int32U uPosition = (Int32U)Position;

	// Send PDO
	MemZero16(PDOMessage, 8);
	//
	PDOMessage[0] = 0x33;
	PDOMessage[1] = 0x01;
	//
	PDOMessage[4] = uPosition & 0xff;
	PDOMessage[5] = (uPosition >> 8) & 0xff;
	PDOMessage[6] = (uPosition >> 16) & 0xff;
	PDOMessage[7] = (uPosition >> 24) & 0xff;
	//
	CANopen_PdoWr(&DEVICE_CANopen_Interface, PDOMessage);
}
// ----------------------------------------

void CLAMP_GoToPosition_mm(Boolean EnableController, Int32S Position)
{
	// Enable controller
	if (EnableController) CLAMP_EnableController(TRUE);

	CLAMP_GoToPosition(Position * DataTable[REG_GEAR_RATIO_K_N] * (MM_TO_INCREMENT / DataTable[REG_GEAR_RATIO_K_D]));
}
// ----------------------------------------

Int32S CLAMP_CurrentPosition()
{
	Int32S Result = 0;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5b45, 3, (pInt32U)(&Result));

	return Result;
}
// ----------------------------------------

Int32S CLAMP_CurrentIncrements()
{
	Int32S Result = 0;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5cbd, 5, (pInt32U)(&Result));

	return Result;
}
// ----------------------------------------

Boolean CLAMP_IsTargetReached()
{
	Int32U StatusWord = 0;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5b26, 0, &StatusWord);

	if ((StatusWord & BIT17) == 0)
		return FALSE;
	else
		return TRUE;
}
// ----------------------------------------

Boolean CLAMP_IsPositionReached()
{
	Int32U StatusWord = 0;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5b26, 0, &StatusWord);

	if ((StatusWord & BIT18) == 0)
		return FALSE;
	else
		return TRUE;
}
// ----------------------------------------

Boolean CLAMP_IsStopLimitReached()
{
	Boolean Result = FALSE;
	Int32S CurrentPosition = CLAMP_CurrentPosition();

	if (ABS(CurrentPosition - PreviousPosition) < STOP_DETECTION_TICKS)
		Result = TRUE;
	PreviousPosition = CurrentPosition;

	return Result;
}
// ----------------------------------------

Boolean CLAMP_IsQSPActive()
{
	Int32U Data = 0;
	CANopen_SdoRd(&DEVICE_CANopen_Interface, 0x5f60, 0, &Data);

	return Data;
}
// ----------------------------------------

_iq CLAMP_ReadForce()
{
	_iq force, force2, result;
	Int16S P2, P0;
	Int16U P1, ADCdata;

	P2 = (Int16S)DataTable[REG_FORCE_FINE_P2];
	P1 = DataTable[REG_FORCE_FINE_P1];
	P0 = (Int16S)DataTable[REG_FORCE_FINE_P0];

	ADCdata = ZbAnanlogInput_GetResult();
	ADCdata = (ADCdata > DataTable[REG_ADC_INIT_OFFSET]) ? (ADCdata - DataTable[REG_ADC_INIT_OFFSET]) : 0;

	// Recalculate analog data to force
	force = _IQmpy(_IQI(ADCdata), _FPtoIQ2(DataTable[REG_FORCE_K_N], DataTable[REG_FORCE_K_D]));
	force2 = _IQdiv(force, _IQ(1000.0f));

	// Fine tuning of the force value
	result = _IQmpy(force2, _IQmpyI32(force2, P2)) + _IQmpy(force, _FPtoIQ2(P1, 1000)) + _IQI(P0);

	// Write to register
	DataTable[REG_FORCE_RESULT] = (result > 0) ? _IQint(_IQdiv(result, _IQ(100))) : 0;

	return (result > 0) ? result : 0;
}
// ----------------------------------------

void CLAMP_ReadADCOffset()
{
	DataTable[REG_DBG_ADC_RAW_DATA] = ZbAnanlogInput_GetResult();
}
// ----------------------------------------

// No more.
