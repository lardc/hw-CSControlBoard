// ----------------------------------------
// Device object dictionary
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H


// ACTIONS
//
#define ACT_CLR_FAULT				3	// Clear fault
#define ACT_CLR_WARNING				4	// Clear warning
#define ACT_CLR_HALT				5	// Clear halt state

#define ACT_HOMING					100	// Start homing
#define ACT_GOTO_POSITION			101 // Go to manually configured position
#define ACT_START_CLAMPING			102 // Star clamping
// 103
#define ACT_RELEASE_CLAMPING		104 // Perform unclamp
#define ACT_HALT					105 // Abort operation
// 106 - 107
#define ACT_SET_TEMPERATURE			108	// Set temperature
#define ACT_RELEASE_ADAPTER			109	// Release adapter for changing
#define ACT_HOLD_ADAPTER			110	// Hold adapter

#define ACT_DBG_READ_TRM_TEMP		115	// Read actual temperature value from TRM
#define ACT_DBG_READ_TRM_POWER		116	// Read TRM output power
#define ACT_DBG_TRM_START			117	// Start TRM operation
#define ACT_DBG_TRM_STOP			118	// Stop TRM operation
#define	ACT_DBG_CONNECT_CONTROL		119	// Connect control circuit to device
#define	ACT_DBG_DISCONNECT_CONTROL	120	// Disconnect control circuit from device
#define ACT_DBG_MOTOR_START			121	// ������ ����������� �������� �������
#define ACT_DBG_MOTOR_STOP			122	// ��������� ����������� �������� �������

#define ACT_SAVE_TO_ROM				200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM		201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT		202	// Reset parameters to default values (only in controller memory)
#define ACT_LOCK_NV_AREA			203	// Lock modifications of parameters area
#define ACT_UNLOCK_NV_AREA			204	// Unlock modifications of parameters area (password-protected)

#define ACT_BOOT_LOADER_REQUEST		320	// Request reboot to bootloader

// REGISTERS
//
#define REG_CLAMP_HEIGHT_CASE_A2	0	// ������ ������� ������� ��� ������� �2 (��)
#define REG_CLAMP_HEIGHT_CASE_B0	1	// ������ ������� ������� ��� ������� B0 (��)
#define REG_CLAMP_HEIGHT_CASE_C1	2	// ������ ������� ������� ��� ������� C1 (��)
#define REG_CLAMP_HEIGHT_CASE_D		3	// ������ ������� ������� ��� ������� D (��)
#define REG_CLAMP_HEIGHT_CASE_E		4	// ������ ������� ������� ��� ������� E (��)
#define REG_CLAMP_HEIGHT_CASE_F		5	// ������ ������� ������� ��� ������� F (��)

#define REG_POS_SPEED_MIN			10	// ����������� �������� ����������� ��� ���������������� (��/���)
#define REG_POS_SPEED_LOW			11	// ���������� �������� ����������� ��� ���������������� (��/���)
#define REG_POS_SPEED_MAX			12	// ������������ �������� ����������� ��� ���������������� (��/���)
#define REG_SLOW_DOWN_DIST			13	// ���������� �� ��������� ����� ��� ������ ���������� (��)
#define REG_HOMING_SPEED			14	// �������� �������� (��/���)
#define REG_HOMING_OFFSET			15	// ������ �������� (��)
#define REG_CLAMP_SPEED_MIN			16	// ����������� �������� ����������� ��� ���������������� (��/���)
#define REG_CLAMP_SPEED_LOW			17	// ���������� �������� ����������� ��� ���������������� (��/���)
#define REG_CLAMP_SPEED_MAX			18	// ������������ �������� ����������� ��� ���������������� (��/���)

#define REG_USE_HEATING				20	// ���������/���������� ��������� ������ ������� �������
#define REG_USE_SAFETY_SENSOR		21	// ���������/���������� ��������� ������� ������������
#define REG_USE_TOOLING_SENSOR		22	// ���������/���������� ��������� ������� ��������
//
// ----------------------------------------
//
#define REG_CUSTOM_POS				64	// Mannually configured position (in mm)

#define REG_DEV_CASE				71	// ��� ������� �������
#define REG_TEMP_SETPOINT			72	// ������� ����������� (� �10)
//
#define REG_DBG_TRM_ADDRESS			84	// ����� ��� �� RS485
//
#define REG_DBG_STEP_DIV			90	// ����������� ������� ����� � ���������� ������
#define REG_DBG_STEPS_MAX			91	// ���������� ����� ��� �������� � ���������� ������
//
#define REG_PWD_1					91	// Unlock password location 1
#define REG_PWD_2					92	// Unlock password location 2
#define REG_PWD_3					93	// Unlock password location 3
#define REG_PWD_4					94	// Unlock password location 4
//
#define REG_SP__2					95
//
// ----------------------------------------
//
#define REG_DEV_STATE				96	// Device state
#define REG_FAULT_REASON			97	// Fault reason in the case DeviceState -> FAULT
#define REG_DISABLE_REASON			98	// Fault reason in the case DeviceState -> DISABLED
#define REG_WARNING					99	// Warning if present
#define REG_PROBLEM					100	// Problem if present
#define REG_TEMP_CH1				101	// Sampled temperature on channel 1

#define REG_TRM_DATA				103	// Data read from TRM
#define REG_TRM_ERROR				104	// TRM error value

#define REG_SAFETY_SENSOR			106	// ��������� ������� ������������
#define REG_TOOLING_SENSOR			107	// ��������� ������� �������� ��������
#define REG_HOMING_SENSOR			108	// ��������� ������� ��������
//
#define REG_CANA_BUSOFF_COUNTER		120 // Counter of bus-off states
#define REG_CANA_STATUS_REG			121	// CAN status register (32 bit)
#define REG_CANA_STATUS_REG_32		122
#define REG_CANA_DIAG_TEC			123	// CAN TEC
#define REG_CANA_DIAG_REC			124	// CAN REC
//
#define REG_SP__3					159
//
// ----------------------------------------
//
#define REG_FWINFO_SLAVE_NID		256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID		257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN			260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN		261	// Begining of the information string record
//

// ENDPOINTS
//

// FAULT CODES
//
#define FAULT_NONE					0	// No fault
//
#define FAULT_THERMOSYSTEM			2	// Thermosystem fault
#define FAULT_TRM					4	// TRM communication fault
#define FAULT_PRESSURE				5	// Pressure is low
#define FAULT_SLIDING				6	// Sliding system fault
//
#define FAULT_SAFETY				7
#define FAULT_POWER_CON				8

// PROBLEM CODES
//
#define PROBLEM_NONE				0	// No problem

// DISABLE CODES
//
#define DISABLE_NONE				0	// No fault
#define DISABLE_BAD_CLOCK			1001	// Problem with main oscillator

// WARNING CODES
//
#define WARNING_NONE				0	// No warning
#define WARNING_WATCHDOG_RESET		1001	// System has been reseted by WD

// USER ERROR CODES
//
#define ERR_NONE					0	// No error
#define ERR_CONFIGURATION_LOCKED	1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED		2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY		3	// Device isn't ready to switch state
#define ERR_WRONG_PWD				4	// Wrong password - unlock failed
//
#define ERR_TRM_COMM_ERR			7	// Communication with TRM failed

#endif // __DEV_OBJ_DIC_H
