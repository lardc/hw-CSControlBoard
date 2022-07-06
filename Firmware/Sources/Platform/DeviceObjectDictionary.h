// -----------------------------------------
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
#define ACT_INHIBIT					109	// Inhibit motor
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
#define ACT_DBGCAN_LOG_SINGLE		149
//
#define ACT_DBGCAN_HEADS_DOWN		150
#define ACT_DBGCAN_HEADS_UP			151
#define ACT_DBGCAN_BEER_OPEN		152
#define ACT_DBGCAN_BEER_CLOSE		153
#define ACT_DBGCAN_CO2_OPEN			154
#define ACT_DBGCAN_CO2_CLOSE		155
#define ACT_DBGCAN_ASYNC_MTR_START	156
#define ACT_DBGCAN_ASYNC_MTR_STOP	157
#define ACT_DBGCAN_SEAMER_PUSH_UP	158
#define ACT_DBGCAN_SEAMER_PUSH_DOWN	159
//
#define ACT_DBG_CAN_EXEC_POURING	160
#define ACT_DBG_CAN_EXEC_SEAMING	161
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
//
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
//
#define REG_BEER_POURING_TIME		34	// Время налива пива, мс
#define REG_CO2_POURING_TIME		35	// Время налива CO2, мс
#define REG_H_DOWN_TO_CO2_PAUSE		36	// Длительность паузы между опусканием голов и началом подачи СО2, мс
#define REG_CO2_TO_BEER_PAUSE		37	// Длительность паузы между остановкой подачи СО2 и подачей пива, мс
#define REG_BEER_TO_H_UP_PAUSE		38	// Длительность паузы между остановкой подачи пива и подъёмом голов, мс (может быть отрицательной)
#define REG_STOP_BEER_BY_SENSOR		39	// Остановка налива по сенсору
#define REG_SEAMER_PUSH_UP_TIME		40	// Задержка после начала зажатия, мс
#define REG_SEAMER_SPIN_UP_TIME		41	// Задержка после начала раскрутки банки, мс
#define REG_SEAMER_STAGE1_TIME		42	// Задержка после выхода валка на этап1 закатки, мс
#define REG_SEAMER_STAGE2_TIME		43	// Задержка после выхода валка на этап2 закатки, мс
#define REG_SEAMER_SLOW_DOWN_TIME	44	// Задержка после начала раскрутки банки, мс
//
#define REG_SEAMER_STAGE1_POS		47
#define REG_SEAMER_STAGE1_POS_32	48
#define REG_SEAMER_STAGE2_POS		49
#define REG_SEAMER_STAGE2_POS_32	50
//
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
//
#define REG_SP__1					63
//
// ----------------------------------------
//
#define REG_CUSTOM_POS				64	// Mannually configured position (in increments)
#define REG_CUSTOM_POS_32			65
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
#define REG_DBG_CAN_MONITOR_DELAY	88
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
#define REG_MASTER_STATE			106	// Device master state
//
#define REG_FORCE_RESULT			110	// Actual force value (in kN x10)
#define REG_DRV_ERROR				111	// Lenze error
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
#define REG_CANO_ERR_CODE			130	// CANopen error code
#define REG_CANO_ERR_INDEX			131	// CANopen error index
#define REG_CANO_ERR_SUBINDEX		132	// CANopen error subindex
#define REG_CANO_ERR_DATA			133	// CANopen error data (low bits)
#define REG_CANO_ERR_DATA_32		134	// CANopen error data (high bits)
//
#define REG_SP__3					159

// ENDPOINTS
//
#define EP16_Data_Time				1
#define EP16_Data_Torque			2
#define EP16_RS485_Rx				3
//
#define EP32_Data_Position			1

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
