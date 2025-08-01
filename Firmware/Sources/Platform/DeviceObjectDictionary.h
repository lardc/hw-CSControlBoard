﻿// -----------------------------------------
// Device object dictionary
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H


// ACTIONS
//
#define ACT_ENABLE_POWER			1	// Enable
#define ACT_DISABLE_POWER			2	// Disable
#define ACT_CLR_FAULT				3	// Clear fault
#define ACT_CLR_WARNING				4	// Clear warning
#define ACT_CLR_HALT				5	// Clear halt state
//
#define ACT_HOMING					100	// Start homing
#define ACT_GOTO_POSITION			101 // Go to manually configured position
#define ACT_START_CLAMPING			102 // Star clamping
#define ACT_CLAMPING_UPDATE			103 // Update regulator with new parameters
#define ACT_RELEASE_CLAMPING		104 // Perform unclamp
#define ACT_HALT					105 // Abort operation
#define ACT_SLIDING_PUSH_OUT		106	// Push out sliding system
#define ACT_SLIDING_PUSH_IN			107	// Push in sliding system
#define ACT_SET_TEMPERATURE			108	// Set temperature
//
#define ACT_DBG_READ_LENZE_REG		110	// Read Lenze register
#define ACT_DBG_WRITE_DAC_RAW		111	// Write raw value to DAC channel
#define ACT_DBG_WRITE_DAC_TEMP		112	// Write temperature value to DAC channel (with correction)
#define ACT_DBG_READ_TEMP			113	// Read temperature value
#define ACT_DBG_READ_TEMP_RAW		114	// Read temperature raw ADC value
#define ACT_DBG_READ_TRM_TEMP		115	// Read actual temperature value from TRM
#define ACT_DBG_READ_TRM_POWER		116	// Read TRM output power
#define ACT_DBG_TRM_START			117	// Start TRM operation
#define ACT_DBG_TRM_STOP			118	// Stop TRM operation
#define ACT_DBG_CON_POWER_SW		119	// Connect pneumatic power switch
#define ACT_DBG_DISCON_POWER_SW		120	// Disconnect pneumatic power switch
#define ACT_DBG_WRITE_LENZE_REG		121	// Write Lenze register
#define ACT_DBG_READ_RAW_ADC		122	// Read raw ADC data
#define ACT_DBG_BREAK_MAN_RLS_ON	123	// Enable brake manual release
#define ACT_DBG_BREAK_MAN_RLS_OFF	124	// Disable brake manual release
#define ACT_DBG_BREAK_AUTO_CONTROL	125	// Disable brake manual control
#define ACT_DBG_READ_FORCE			126 // Read current force value
#define ACT_DBG_READ_LENZE_ERROR	127 // Read current error value
// 128 - 129
#define ACT_DBG_SLS_PUSH_UP			130	// Push up sliding system
#define ACT_DBG_SLS_PUSH_DOWN		131	// Push down sliding system
#define ACT_DBG_SLS_PUSH_OUT		132	// Push out sliding system
#define ACT_DBG_SLS_PUSH_IN			133	// Push in sliding system
//
#define ACT_SAVE_TO_ROM				200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM		201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT		202	// Reset parameters to default values (only in controller memory)
#define ACT_LOCK_NV_AREA			203	// Lock modifications of parameters area
#define ACT_UNLOCK_NV_AREA			204	// Unlock modifications of parameters area (password-protected)
//
#define ACT_BOOT_LOADER_REQUEST		320	// Request reboot to bootloader


// REGISTERS
//
#define REG_ADC_INIT_OFFSET			0	// Initial ADC offset (in ticks of 4095)

// Конфигурация аппаратных особенностей блока
#define REG_NO_HALT_ON_QUICK_STOP	1	// Игнорирование аппаратной остановки ПЧ (не приводит к переходу прошивки в состояние DS_Halt)
#define REG_INVERT_DI6_LENZE_INPUT	2	// Инвертирование сигнала на входе DI6 ПЧ
#define REG_ALTER_MM_TO_INCREMENT	3	// Переопределить коэффициент пересчёта перемещения (мм) в тики
#define REG_BOARD_VERSION			4	// Включить специфические функции для плат версии:
										// 10 - версия 1.0
										// 11 - версия 1.1
										// Все остальные значения трактуются как версия 2.0
#define REG_MAX_ALLOWED_FORCE		5	// Переопределить максимальное разрешённое усилие (в кН х10)

#define REG_FORCE_K_N				10	// Force recalculating K coefficient (N)
#define REG_FORCE_K_D				11	// Force recalculating K coefficient (D)
#define REG_FORCE_FINE_P2			12	// Force quadratic fine tuning P2 x1e6
#define REG_FORCE_FINE_P1			13	// Force quadratic fine tuning P1 x1000
#define REG_FORCE_FINE_P0			14	// Force quadratic fine tuning P0 (in N)
#define REG_HOMING_REF_SPEED		15	// Homing ref. initial speed (in units / 16384)
#define REG_HOMING_SEARCH_SPEED		16	// Homing ref. search speed (in units / 16384)
#define REG_HOMING_OFFSET			17	// Homing offset (in units / 16384)
#define REG_HOMING_TORQUE_LIMIT		18	// Homing torque limit (in %)
#define REG_POSITION_SPEED_LIMIT	19	// Positioning: Speed limit (in %)
#define REG_POSITION_TORQUE_LIMIT	20	// Positioning: Torque limit (in %)
#define REG_CLAMP_SPEED_LIMIT		21	// Clamping: Speed limit (in %)
#define REG_CLAMP_TORQUE_LIMIT		22	// Clamping: Torque limit (in %)
#define REG_CLAMP_TIMEOUT			23	// Clamping: Clamp timeout to reach force (in ms) (must be a multiple of 50ms @ 20 Hz TIMER2)
#define REG_CLAMP_ERR_ZONE			24	// Clamping: Error dead zone (in %)
#define REG_MAX_CONT_FORCE			25	// Maximum force for continious operating (in kN x10)
#define REG_MAX_CONT_FORCE_TIMEOUT	26	// Timeout for automatic release at maximum force (in ms)
#define REG_CLAMPING_RLS_POS		27	// Release target position (in mm)
#define REG_CLAMPING_DEV_OFFSET		28	// Device safe offset (in mm)
#define REG_FORCE_Kp_N				29	// PID Kp (N)
#define REG_FORCE_Kp_D				30	// PID Kp (D)
#define REG_FORCE_Kp_POST_N			31	// PID Kp post regulation (N)
#define REG_FORCE_Kp_POST_D			32	// PID Kp post regulation (D)
#define REG_CONTINUOUS_CTRL			33	// Enable controller after reaching desired force
#define REG_TEMP1_READ_FINE_P2		34	// Temperature channel 1 read p2 member of quadratic correction polynom (x10^6)
#define REG_TEMP1_READ_FINE_P1		35	// Temperature channel 1 read p1 member of quadratic correction polynom (x1000)
#define REG_TEMP1_READ_FINE_P0		36	// Temperature channel 1 read p0 member of quadratic correction polynom (offset) (in C x10)
#define REG_TEMP2_READ_FINE_P2		37	// Temperature channel 2 read p2 member of quadratic correction polynom (x10^6)
#define REG_TEMP2_READ_FINE_P1		38	// Temperature channel 2 read p1 member of quadratic correction polynom (x1000)
#define REG_TEMP2_READ_FINE_P0		39	// Temperature channel 2 read p0 member of quadratic correction polynom (offset) (in C x10)
#define REG_TEMP1_TO_DAC_FINE_P2	40	// Temperature channel 1 to DAC p2 member of quadratic correction polynom (x10^6)
#define REG_TEMP1_TO_DAC_FINE_P1	41	// Temperature channel 1 to DAC p1 member of quadratic correction polynom (x1000)
#define REG_TEMP1_TO_DAC_FINE_P0	42	// Temperature channel 1 to DAC p0 member of quadratic correction polynom (offset) (in C x10)
#define REG_TEMP2_TO_DAC_FINE_P2	43	// Temperature channel 2 to DAC p2 member of quadratic correction polynom (x10^6)
#define REG_TEMP2_TO_DAC_FINE_P1	44	// Temperature channel 2 to DAC p1 member of quadratic correction polynom (x1000)
#define REG_TEMP2_TO_DAC_FINE_P0	45	// Temperature channel 2 to DAC p0 member of quadratic correction polynom (offset) (in C x10)
#define REG_USE_HEATING				46	// Enable/disable heating system
#define REG_TEMP_READ_K_N			47	// Temperature read recalculating K coefficient (N)
#define REG_TEMP_READ_K_D			48	// Temperature read recalculating K coefficient (D)
#define REG_TEMP_TO_DAC_K_N			49	// Temperature to DAC recalculating K coefficient (N)
#define REG_TEMP_TO_DAC_K_D			50	// Temperature to DAC recalculating K coefficient (D)
#define REG_2ST_FORCE_LIM			51	// Double stage clamping force threshold (in x10 kN)
#define REG_USE_2ST_CLAMP			52	// Enable/disable double stage clamping
#define REG_USE_SLIDING_SENSOR		53	// Enable/disable sliding system sensor
#define REG_USE_CLAMP_BREAK			54	// Enable/disable motor break control
#define REG_GEAR_RATIO_K_N			55	// Gear ratio coefficient (N)
#define REG_GEAR_RATIO_K_D			56	// Gear ratio coefficient (D)
#define REG_BALL_SCREW_STROKE		57	// Ball screw overall move (in mm)
#define REG_ALLOWED_MOVE			58	// Clamp allowed move (in mm)
#define REG_POWER_SW_DELAY			59	// Time to close power switch (in ms) (must be a multiple of 50ms @ 20 Hz TIMER2)
#define REG_USE_AIR_CONTROL			60	// Use air pressure monitoring system
#define REG_FORCE_SET_K				61	// Force setpoint K coefficient (x1000)
#define REG_FORCE_SET_B				62	// Force setpoint B coefficient (in x10 kN)
//
#define REG_SP__1					63
//
// ----------------------------------------
//
#define REG_CUSTOM_POS				64	// Mannually configured position (in mm)
#define REG_FORCE_VAL				70	// Force value (in kN x10)
#define REG_DEV_HEIGHT				71	// Device height (in mm)
#define REG_TEMP_SETPOINT			72	// Temperature setpoint (in C x10)
#define REG_USE_SLIDING_SYSTEM		73	// Activate sliding system
//
#define REG_DBG_CAN_INDEX			80	// Read\Write Lenze register - Index
#define REG_DBG_CAN_SUBCODE			81	// Read\Write Lenze register - Subcode
#define REG_DBG_TEMP_CH_INDEX		82	// Channel select
#define REG_DBG_TEMP_CH_DATA		83	// DAC data to write
#define REG_DBG_TRM_ADDRESS			84	// TRM address
#define REG_DBG_PAUSE_T_FEEDBACK	85	// Deactivate temperature feedback
#define REG_DBG_CAN_DATA			86	// Read\Write Lenze register - Data
#define REG_DBG_CAN_DATA_32			87
//
#define REG_PWD_1					91	// Unlock password loct   ation 1
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
//
#define REG_TEMP_CH1				101	// Sampled temperature on channel 1
#define REG_TEMP_CH2				102	// Sampled temperature on channel 2
#define REG_TRM_DATA				103	// Data read from TRM
#define REG_TRM_ERROR				104	// TRM error value
#define REG_SLIDING_SENSOR			105	// Sliding sensor current state
//
#define REG_FORCE_RESULT			110	// Actual force value (in kN x10)
#define REG_DRV_ERROR				111	// Lenze error
#define REG_QUIK_STOP_STATUS		112	// Состояние сигнала QuickStop
//
#define REG_DBG_READ_REG			112	// Read Lenze register output
#define REG_DBG_READ_REG_32			113
#define REG_DBG_TEMP				114	// Temperature isolator data (in C x10)
#define REG_DBG_ADC_RAW_DATA		115	// Initial ADC offset (in ticks of 4095 scale)
#define REG_DBG_TEMP_RAW			117	// Temperature isolator raw data
//
#define REG_CANA_BUSOFF_COUNTER		120 // Counter of bus-off states
#define REG_CANA_STATUS_REG			121	// CAN status register (32 bit)
#define REG_CANA_STATUS_REG_32		122
#define REG_CANA_DIAG_TEC			123	// CAN TEC
#define REG_CANA_DIAG_REC			124	// CAN REC
//
#define REG_CANB_BUSOFF_COUNTER		125 // Counter of bus-off states
#define REG_CANB_STATUS_REG			126	// CAN status register (32 bit)
#define REG_CANB_STATUS_REG_32		127
#define REG_CANB_DIAG_TEC			128	// CAN TEC
#define REG_CANB_DIAG_REC			129	// CAN REC
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

// ENDPOINTS
//
#define EP16_Data_ForceActual		1	// Clamping force value from sensor
#define EP16_Data_ForceDesired		2	// Configured clamping force value
#define EP16_Data_ForceError		3	// Force error
#define EP16_XLog_SubState			4	// Extended logging clamping process substate
#define EP16_XLog_Force				5	// Extended logging force data from sensor
#define EP16_XLog_Error				6	// Extended logging clamping error
#define EP16_XLog_TorqueLimit		7	// Extended logging motor torque limit
//
#define EP32_Data_CtrlIncrements	1	// Input data for position controller
#define EP32_Data_Position			2	// Data from resolver

// FAULT CODES
//
#define FAULT_NONE					0	// No fault
// 1
#define FAULT_THERMOSYSTEM			2	// Thermosystem fault
#define FAULT_CANOPEN				3	// CANopen high-level fault
#define FAULT_TRM					4	// TRM communication fault
#define FAULT_PRESSURE				5	// Pressure is low
#define FAULT_SLIDING				6	// Sliding system fault

// PROBLEM CODES
//
#define PROBLEM_NONE				0	// No problem
#define PROBLEM_NO_FORCE			1	// Force not reached
#define PROBLEM_NO_AIR_PRESSURE		2	// No air pressure for power switch

// DISABLE CODES
//
#define DISABLE_NONE				0	// No fault
#define DISABLE_LENZE_ERROR			1	// Lenze inverter error
//
#define DISABLE_BAD_CLOCK			1001	// Problem with main oscillator

// WARNING CODES
//
#define WARNING_NONE				0	// No warning
//
#define WARNING_WATCHDOG_RESET		1001	// System has been reseted by WD

// USER ERROR CODES
//
#define ERR_NONE					0	// No error
#define ERR_CONFIGURATION_LOCKED	1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED		2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY		3	// Device isn't ready to switch state
#define ERR_WRONG_PWD				4	// Wrong password - unlock failed
#define ERR_SLIDING_SYSTEM			5	// Sliding system not ready
#define ERR_PARAMETER_OUT_OF_RNG	6	// Configured parameter is out of range
#define ERR_TRM_COMM_ERR			7	// Communication with TRM failed
#define ERR_NO_AIR_PRESSURE			8	// Air pressure for power switch too low

#endif // __DEV_OBJ_DIC_H
