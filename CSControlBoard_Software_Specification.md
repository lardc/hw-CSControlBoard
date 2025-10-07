### CSControlBoard — Software Specification

- Branch: `develop-PCBv2.0-MME016`
- Commit: `47f60f7093925e53202f5c996038724e734d0134`
- Build date: `2025/09/30T11:08:06`

### Overview
Clamp control firmware for TMS320F2809. It controls clamping motor (speed/torque/position), regulates force using sensor feedback, monitors temperature and controls heating, operates a pneumatic power switch with air pressure monitoring, and drives a sliding system. Communication interfaces include CAN (internal/external), RS‑232 (SCCI), and SPI peripherals (DAC, TRM, optical sensors). Data streams and extended logs are provided via endpoints.

DataTable layout:
- 0–63: Non-volatile (NV) RW (saved to EPROM)
- 64–95: Volatile RW
- 96–159: Read-only (RO)
- 256+: Firmware information (RO)

All register types are `uint16` unless noted. For “×10” fields, the actual value equals raw/10.

## Writable Registers

### Device Configuration & Calibration (NV 0–63)
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 0 | REG_ADC_INIT_OFFSET | uint16 | Initial ADC offset (ticks of 4095) | 0–4095 |  | ADC offset correction in raw ticks |
| 10 | REG_FORCE_K_N | uint16 | Force recalculation K numerator | 0 - INT16U_MAX | 0 to INT16U_MAX | Force gain numerator (default 2751) |
| 11 | REG_FORCE_K_D | uint16 | Force recalculation K denominator | 1–10000 | 1 to X_D_MAX | Force gain denominator |
| 12 | REG_FORCE_FINE_P2 | uint16 | Force quadratic fine tune P2 ×1e6 | 0 - INT16U_MAX | 0 to INT16U_MAX | Quadratic term p2 (×1e6) |
| 13 | REG_FORCE_FINE_P1 | uint16 | Force quadratic fine tune P1 ×1000 | 0 - INT16U_MAX | 0 to INT16U_MAX | Linear term p1 (×1000, default 1000) |
| 14 | REG_FORCE_FINE_P0 | uint16 | Force quadratic fine tune P0 (N) | 0 - INT16U_MAX | 0 to INT16U_MAX | Offset term p0 (N) |
| 15 | REG_HOMING_REF_SPEED | uint16 | Homing initial speed (units/s) | 100–5000 | HOMING_REF_SPEED_MIN to HOMING_REF_SPEED_MAX | Initial homing speed |
| 16 | REG_HOMING_SEARCH_SPEED | uint16 | Homing search speed (units/s) | 100–5000 | HOMING_SEARCH_SPEED_MIN to HOMING_SEARCH_SPEED_MAX | Search speed while homing |
| 17 | REG_HOMING_OFFSET | uint16 | Homing offset (units) | 100–10000 | HOMING_OFFSET_MIN to HOMING_OFFSET_MAX | Offset after reference found |
| 18 | REG_HOMING_TORQUE_LIMIT | uint16 | Homing torque limit (%) | 5–50 | HOMING_TORQUE_MIN to HOMING_TORQUE_MAX | Max torque allowed during homing |
| 19 | REG_POSITION_SPEED_LIMIT | uint16 | Positioning speed limit (%) | 10–150 | POSITION_SPEED_MIN to POSITION_SPEED_MAX | Speed limit for position moves |
| 20 | REG_POSITION_TORQUE_LIMIT | uint16 | Positioning torque limit (%) | 10–100 | POSITION_TORQUE_MIN to POSITION_TORQUE_MAX | Torque limit for position moves |
| 21 | REG_CLAMP_SPEED_LIMIT | uint16 | Clamping speed limit (%) | 5–50 | CLAMP_SPEED_MIN to CLAMP_SPEED_MAX | Speed limit during clamping |
| 22 | REG_CLAMP_TORQUE_LIMIT | uint16 | Clamping torque limit (%) | 5–199 | CLAMP_TORQUE_LIMIT_MIN to CLAMP_TORQUE_LIMIT_MAX | Torque limit during clamping |
| 23 | REG_CLAMP_TIMEOUT | uint16 | Clamp timeout to reach force (ms) | 1000–60000 | CLAMP_TIMEOUT_MIN to CLAMP_TIMEOUT_MAX | Time budget to reach set force (20 Hz step) |
| 24 | REG_CLAMP_ERR_ZONE | uint16 | Force error dead-zone (%) | 1–20 | CLAMP_ERR_ZONE_MIN to CLAMP_ERR_ZONE_MAX | Allowed relative error band |
| 25 | REG_MAX_CONT_FORCE | uint16 | Max continuous force (kN×10) | 50–1100 | FORCE_VAL_MIN to FORCE_VAL_MAX | Limit to protect mechanism |
| 26 | REG_MAX_CONT_FORCE_TIMEOUT | uint16 | Auto release timeout at max force (ms) | 2000–10000 | AUTO_RLS_TIMEOUT_MIN to AUTO_RLS_TIMEOUT_MAX | Hold time before auto-release |
| 27 | REG_CLAMPING_RLS_POS | uint16 | Release target position (mm) | 0–50 | POS_RLS_OFFS_MIN to POS_RLS_OFFS_MAX | Target position for unclamp |
| 28 | REG_CLAMPING_DEV_OFFSET | uint16 | Device safe offset (mm) | 0–50 | POS_DEV_OFFS_MIN to POS_DEV_OFFS_MAX | Mechanical safety offset |
| 29 | REG_FORCE_Kp_N | uint16 | Force regulator Kp numerator | 0 - INT16U_MAX | 0 to INT16U_MAX | Proportional gain numerator (default 40) |
| 30 | REG_FORCE_Kp_D | uint16 | Force regulator Kp denominator | 1–10000 | 1 to X_D_MAX | Proportional gain denominator |
| 31 | REG_FORCE_Kp_POST_N | uint16 | Post‑regulation Kp numerator | 0 - INT16U_MAX | 0 to INT16U_MAX | Post phase Kp numerator (default 10) |
| 32 | REG_FORCE_Kp_POST_D | uint16 | Post‑regulation Kp denominator | 1–10000 | 1 to X_D_MAX | Post phase Kp denominator |
| 34 | REG_TEMP1_READ_FINE_P2 | uint16 | Temp ch1 read poly p2 (×1e6) | 0 - INT16U_MAX | 0 to INT16U_MAX | Quadratic correction p2 |
| 35 | REG_TEMP1_READ_FINE_P1 | uint16 | Temp ch1 read poly p1 (×1000) | 0 - INT16U_MAX | 0 to INT16U_MAX | Linear correction p1 (default 1000) |
| 36 | REG_TEMP1_READ_FINE_P0 | uint16 | Temp ch1 read poly p0 (C×10) | 0 - INT16U_MAX | 0 to INT16U_MAX | Offset correction p0 |
| 37 | REG_TEMP2_READ_FINE_P2 | uint16 | Temp ch2 read poly p2 (×1e6) | 0 - INT16U_MAX | 0 to INT16U_MAX | Quadratic correction p2 |
| 38 | REG_TEMP2_READ_FINE_P1 | uint16 | Temp ch2 read poly p1 (×1000) | 0 - INT16U_MAX | 0 to INT16U_MAX | Linear correction p1 (default 1000) |
| 39 | REG_TEMP2_READ_FINE_P0 | uint16 | Temp ch2 read poly p0 (C×10) | 0 - INT16U_MAX | 0 to INT16U_MAX | Offset correction p0 |
| 40 | REG_TEMP1_TO_DAC_FINE_P2 | uint16 | Temp ch1→DAC poly p2 (×1e6) | 0 - INT16U_MAX | 0 to INT16U_MAX | Quadratic conversion p2 |
| 41 | REG_TEMP1_TO_DAC_FINE_P1 | uint16 | Temp ch1→DAC poly p1 (×1000) | 0 - INT16U_MAX | 0 to INT16U_MAX | Linear conversion p1 (default 1000) |
| 42 | REG_TEMP1_TO_DAC_FINE_P0 | uint16 | Temp ch1→DAC poly p0 (C×10) | 0 - INT16U_MAX | 0 to INT16U_MAX | Offset conversion p0 |
| 43 | REG_TEMP2_TO_DAC_FINE_P2 | uint16 | Temp ch2→DAC poly p2 (×1e6) | 0 - INT16U_MAX | 0 to INT16U_MAX | Quadratic conversion p2 |
| 44 | REG_TEMP2_TO_DAC_FINE_P1 | uint16 | Temp ch2→DAC poly p1 (×1000) | 0 - INT16U_MAX | 0 to INT16U_MAX | Linear conversion p1 (default 1000) |
| 45 | REG_TEMP2_TO_DAC_FINE_P0 | uint16 | Temp ch2→DAC poly p0 (C×10) | 0 - INT16U_MAX | 0 to INT16U_MAX | Offset conversion p0 |
| 55 | REG_GEAR_RATIO_K_N | uint16 | Gear ratio K numerator | 1–10000 | 1 to X_D_MAX | Mechanical ratio numerator |
| 56 | REG_GEAR_RATIO_K_D | uint16 | Gear ratio K denominator | 1–10000 | 1 to X_D_MAX | Mechanical ratio denominator |
| 57 | REG_BALL_SCREW_STROKE | uint16 | Ball screw overall stroke (mm) | 0–120 | 0 to POS_MAX | Mechanical stroke length |
| 58 | REG_ALLOWED_MOVE | uint16 | Clamp allowed move (mm) | 0–120 | 0 to POS_MAX | Allowed travel in current setup |
| 61 | REG_FORCE_SET_K | uint16 | Force setpoint K (×1000) | 100–10000 | X_D_DEF1 to X_D_DEF3 | Scaling for force setpoint |

### Safety & Monitoring Control (NV 0–63)
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 33 | REG_CONTINUOUS_CTRL | uint16 | Enable controller after force reached | 0–1 | DISABLE/ENABLE | Keep regulator active after reaching set force |
| 46 | REG_USE_HEATING | uint16 | Enable/disable heating system | 0–1 | DISABLE/ENABLE | Heating controller on/off |
| 52 | REG_USE_2ST_CLAMP | uint16 | Enable double‑stage clamping | 0–1 | DISABLE/ENABLE | Two‑stage clamping on/off |
| 53 | REG_USE_SLIDING_SENSOR | uint16 | Use sliding system sensor | 0–1 | DISABLE/ENABLE | Enable sliding sensor input |
| 54 | REG_USE_CLAMP_BREAK | uint16 | Use motor brake control | 0–1 | DISABLE/ENABLE | Enable brake output control |
| 59 | REG_POWER_SW_DELAY | uint16 | Pneumatic power switch close time (ms) | 0 - INT16U_MAX | 0 to INT16U_MAX | Time to close power switch (20 Hz step) |
| 60 | REG_USE_AIR_CONTROL | uint16 | Use air pressure monitoring | 0–1 | DISABLE/ENABLE | Enable low‑pressure detection |

### Runtime Control Parameters (RW 64–95)
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 64 | REG_CUSTOM_POS | uint16 | Manually configured position (mm) | 0–120 | 0 to POS_MAX | Target position for manual moves |
| 70 | REG_FORCE_VAL | uint16 | Force setpoint (kN×10) | 50–1100 | FORCE_VAL_MIN to FORCE_VAL_MAX | Desired clamping force |
| 71 | REG_DEV_HEIGHT | uint16 | Device height (mm) | 0–70 | DEV_HEIGHT_MIN to DEV_HEIGHT_MAX | Height of device under clamp |
| 72 | REG_TEMP_SETPOINT | uint16 | Temperature setpoint (C×10) | 0–2000 | TRM_TEMP_MIN to TRM_TEMP_MAX | Target temperature |
| 73 | REG_USE_SLIDING_SYSTEM | uint16 | Activate sliding system | 0–1 | DISABLE/ENABLE | Engage sliding mechanism |

### Debug Parameters & Service (RW 64–95)
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 80 | REG_DBG_CAN_INDEX | uint16 | Lenze register index (debug) | 0 - INT16U_MAX | 0 to INT16U_MAX | Target register index |
| 81 | REG_DBG_CAN_SUBCODE | uint16 | Lenze register subcode (debug) | 0 - INT16U_MAX | 0 to INT16U_MAX | Target subcode |
| 82 | REG_DBG_TEMP_CH_INDEX | uint16 | Temperature channel select | 1–2 | CH1/CH2 | Select channel 1 or 2 |
| 83 | REG_DBG_TEMP_CH_DATA | uint16 | DAC data to write (debug) | 0 - INT16U_MAX | 0 to INT16U_MAX | Raw DAC data value |
| 84 | REG_DBG_TRM_ADDRESS | uint16 | TRM address | 0–255 |  | External TRM device address |
| 85 | REG_DBG_PAUSE_T_FEEDBACK | uint16 | Pause temperature feedback | 0–1 | RUN/PAUSE | Run/stop temperature loop |
| 86 | REG_DBG_CAN_DATA | uint16 | Lenze register data (debug) | 0 - INT16U_MAX | 0 to INT16U_MAX | Data payload |
| 87 | REG_DBG_CAN_DATA_32 | uint16 | Lenze register data (debug, 32‑bit part) | 0 - INT16U_MAX | 0 to INT16U_MAX | 32‑bit value part |

### Security & Locking (RW 64–95)
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 91 | REG_PWD_1 | uint16 | Unlock password word 1 | 0 - INT16U_MAX | 0 to INT16U_MAX | Unlock key (word 1) |
| 92 | REG_PWD_2 | uint16 | Unlock password word 2 | 0 - INT16U_MAX | 0 to INT16U_MAX | Unlock key (word 2) |
| 93 | REG_PWD_3 | uint16 | Unlock password word 3 | 0 - INT16U_MAX | 0 to INT16U_MAX | Unlock key (word 3) |
| 94 | REG_PWD_4 | uint16 | Unlock password word 4 | 0 - INT16U_MAX | 0 to INT16U_MAX | Unlock key (word 4) |

## Read-Only Registers

### Device Status (96–100)
| Register | Num | Value | Value Name | Value Description |
|----------|-----|-------|------------|-------------------|
| REG_DEV_STATE | 96 | 0 | DS_None | No active state |
|  |  | 1 | DS_Fault | Fault state (latched fault) |
|  |  | 2 | DS_Disabled | Disabled due to error or request |
|  |  | 3 | DS_Ready | Ready/idle state |
|  |  | 4 | DS_Halt | Halted by user or safety |
|  |  | 5 | DS_Homing | Homing procedure in progress |
|  |  | 6 | DS_Position | Moving to target position |
|  |  | 7 | DS_Clamping | Clamping in progress |
|  |  | 8 | DS_ClampingDone | Force reached, post‑regulation phase |
|  |  | 9 | DS_ClampingUpdate | Applying updated clamping parameters |
|  |  | 10 | DS_ClampingRelease | Unclamping in progress |
|  |  | 11 | DS_Sliding | Sliding system operation |
| REG_FAULT_REASON | 97 | 0 | FAULT_NONE | No fault |
|  |  | 2 | FAULT_THERMOSYSTEM | Thermosystem fault |
|  |  | 3 | FAULT_CANOPEN | CANopen high‑level fault |
|  |  | 4 | FAULT_TRM | TRM communication fault |
|  |  | 5 | FAULT_PRESSURE | Air pressure too low |
|  |  | 6 | FAULT_SLIDING | Sliding system fault |
| REG_DISABLE_REASON | 98 | 0 | DISABLE_NONE | Not disabled |
|  |  | 1 | DISABLE_LENZE_ERROR | Lenze inverter error |
|  |  | 1001 | DISABLE_BAD_CLOCK | Main oscillator problem |
| REG_WARNING | 99 | 0 | WARNING_NONE | No warnings |
|  |  | 1001 | WARNING_WATCHDOG_RESET | System was reset by watchdog |
| REG_PROBLEM | 100 | 0 | PROBLEM_NONE | No problems |
|  |  | 1 | PROBLEM_NO_FORCE | Desired force not reached |
|  |  | 2 | PROBLEM_NO_AIR_PRESSURE | Air pressure for power switch too low |
|  |  | 3 | PROBLEM_NO_CLAMPING | Clamping not detected |

### Sensors, Telemetry & Diagnostics
| Num | Name | Type | Description | Value List/Range | Value Name | Value Description |
|-----|------|------|-------------|------------------|------------|-------------------|
| 101 | REG_TEMP_CH1 | uint16 | Temperature channel 1 (C×10) | 0–2000 | TRM_TEMP_MIN to TRM_TEMP_MAX | Measured temperature Ch1 |
| 102 | REG_TEMP_CH2 | uint16 | Temperature channel 2 (C×10) | 0–2000 | TRM_TEMP_MIN to TRM_TEMP_MAX | Measured temperature Ch2 |
| 103 | REG_TRM_DATA | uint16 | TRM read data | 0 - INT16U_MAX | 0 to INT16U_MAX | Last value from TRM |
| 104 | REG_TRM_ERROR | uint16 | TRM error code | 0 - INT16U_MAX | 0 to INT16U_MAX | TRM communication/status code |
| 105 | REG_SLIDING_SENSOR | uint16 | Sliding sensor state | 0–1 | OFF/ON | Sliding sensor inactive/active |
| 110 | REG_FORCE_RESULT | uint16 | Actual force (kN×10) | 0–1100 |  | Measured clamping force |
| 111 | REG_DRV_ERROR | uint16 | Lenze error | 0 - INT16U_MAX | 0 to INT16U_MAX | Current Lenze inverter error |
| 112 | REG_DBG_READ_REG | uint16 | Lenze register readout | 0 - INT16U_MAX | 0 to INT16U_MAX | Read data (16‑bit) |
| 113 | REG_DBG_READ_REG_32 | uint16 | Lenze register readout (32‑bit part) | 0 - INT16U_MAX | 0 to INT16U_MAX | Read data (32‑bit part) |
| 114 | REG_DBG_TEMP | uint16 | Temperature isolator (C×10) | 0–2000 | TRM_TEMP_MIN to TRM_TEMP_MAX | Temperature from isolator |
| 115 | REG_DBG_ADC_RAW_DATA | uint16 | ADC raw data (ticks of 4095) | 0–4095 |  | Raw ADC sample |
| 117 | REG_DBG_TEMP_RAW | uint16 | Temperature isolator raw | 0 - INT16U_MAX | 0 to INT16U_MAX | Raw sensor data |
| 120 | REG_CANA_BUSOFF_COUNTER | uint16 | CANa bus‑off counter | 0 - INT16U_MAX | 0 to INT16U_MAX | Number of bus‑off events |
| 121 | REG_CANA_STATUS_REG | uint32 | CANa status register (low/high) |  |  | Bitfield status (lo/hi words) |
| 122 | REG_CANA_STATUS_REG_32 | uint16 | CANa status register (second word) |  |  | High word of status |
| 123 | REG_CANA_DIAG_TEC | uint16 | CANa TEC | 0 - INT16U_MAX | 0 to INT16U_MAX | Transmit error counter |
| 124 | REG_CANA_DIAG_REC | uint16 | CANa REC | 0 - INT16U_MAX | 0 to INT16U_MAX | Receive error counter |
| 125 | REG_CANB_BUSOFF_COUNTER | uint16 | CANb bus‑off counter | 0 - INT16U_MAX | 0 to INT16U_MAX | Number of bus‑off events |
| 126 | REG_CANB_STATUS_REG | uint32 | CANb status register (low/high) |  |  | Bitfield status (lo/hi words) |
| 127 | REG_CANB_STATUS_REG_32 | uint16 | CANb status register (second word) |  |  | High word of status |
| 128 | REG_CANB_DIAG_TEC | uint16 | CANb TEC | 0 - INT16U_MAX | 0 to INT16U_MAX | Transmit error counter |
| 129 | REG_CANB_DIAG_REC | uint16 | CANb REC | 0 - INT16U_MAX | 0 to INT16U_MAX | Receive error counter |
| 256 | REG_FWINFO_SLAVE_NID | uint16 | Device CAN slave node ID | 0 - INT16U_MAX | 0 to INT16U_MAX | Slave CAN node ID |
| 257 | REG_FWINFO_MASTER_NID | uint16 | Device CAN master node ID | 0 - INT16U_MAX | 0 to INT16U_MAX | Master CAN node ID |
| 260 | REG_FWINFO_STR_LEN | uint16 | Firmware info string length | 0 - INT16U_MAX | 0 to INT16U_MAX | String length in bytes |
| 261 | REG_FWINFO_STR_BEGIN | uint16 | Firmware info string buffer |  |  | String data (words) |

## Commands

### Power & System Control
| Num | Name | Description |
|-----|------|-------------|
| 1 | ACT_ENABLE_POWER | Enable power (standby/ready) |
| 2 | ACT_DISABLE_POWER | Disable power (turn off) |
| 3 | ACT_CLR_FAULT | Clear fault state |
| 4 | ACT_CLR_WARNING | Clear warnings and problems |
| 5 | ACT_CLR_HALT | Clear halt and return to ready |

### Motion & Clamping Control
| Num | Name | Description |
|-----|------|-------------|
| 100 | ACT_HOMING | Start homing procedure |
| 101 | ACT_GOTO_POSITION | Go to manual position |
| 102 | ACT_START_CLAMPING | Start clamping cycle |
| 103 | ACT_CLAMPING_UPDATE | Apply updated regulation parameters |
| 104 | ACT_RELEASE_CLAMPING | Perform unclamp |
| 105 | ACT_HALT | Abort operation and halt |
| 106 | ACT_SLIDING_PUSH_OUT | Push out sliding system |
| 107 | ACT_SLIDING_PUSH_IN | Push in sliding system |

### Temperature & Debug
| Num | Name | Description |
|-----|------|-------------|
| 108 | ACT_SET_TEMPERATURE | Set temperature target |
| 110 | ACT_DBG_READ_LENZE_REG | Read Lenze register |
| 111 | ACT_DBG_WRITE_DAC_RAW | Write raw value to DAC |
| 112 | ACT_DBG_WRITE_DAC_TEMP | Write temperature value to DAC (with correction) |
| 113 | ACT_DBG_READ_TEMP | Read temperature value |
| 114 | ACT_DBG_READ_TEMP_RAW | Read temperature raw ADC |
| 115 | ACT_DBG_READ_TRM_TEMP | Read TRM actual temperature |
| 116 | ACT_DBG_READ_TRM_POWER | Read TRM output power |
| 117 | ACT_DBG_TRM_START | Start TRM operation |
| 118 | ACT_DBG_TRM_STOP | Stop TRM operation |
| 119 | ACT_DBG_CON_POWER_SW | Connect pneumatic power switch |
| 120 | ACT_DBG_DISCON_POWER_SW | Disconnect pneumatic power switch |
| 121 | ACT_DBG_WRITE_LENZE_REG | Write Lenze register |
| 122 | ACT_DBG_READ_RAW_ADC | Read raw ADC data |
| 123 | ACT_DBG_BREAK_MAN_RLS_ON | Enable brake manual release |
| 124 | ACT_DBG_BREAK_MAN_RLS_OFF | Disable brake manual release |
| 125 | ACT_DBG_BREAK_AUTO_CONTROL | Disable brake manual control |
| 126 | ACT_DBG_READ_FORCE | Read current force value |
| 127 | ACT_DBG_READ_LENZE_ERROR | Read current Lenze error |
| 130 | ACT_DBG_SLS_PUSH_UP | Sliding system push up |
| 131 | ACT_DBG_SLS_PUSH_DOWN | Sliding system push down |
| 132 | ACT_DBG_SLS_PUSH_OUT | Sliding system push out |
| 133 | ACT_DBG_SLS_PUSH_IN | Sliding system push in |

### NV Data Management & Boot
| Num | Name | Description |
|-----|------|-------------|
| 200 | ACT_SAVE_TO_ROM | Save NV area to EPROM |
| 201 | ACT_RESTORE_FROM_ROM | Restore NV area from EPROM |
| 202 | ACT_RESET_TO_DEFAULT | Reset parameters to defaults (RAM) |
| 203 | ACT_LOCK_NV_AREA | Lock NV area modifications |
| 204 | ACT_UNLOCK_NV_AREA | Unlock NV area (password protected) |
| 320 | ACT_BOOT_LOADER_REQUEST | Reboot to bootloader |

## Endpoints (Read‑only)
| Num | Name | Description | Data Type | Size |
|-----|------|-------------|-----------|------|
| 1 | EP16_Data_ForceActual | Clamping force (sensor) | uint16 | 500 |
| 2 | EP16_Data_ForceDesired | Configured clamping force | uint16 | 500 |
| 3 | EP16_Data_ForceError | Force error | uint16 | 500 |
| 4 | EP16_XLog_SubState | Extended logging: clamping sub‑state | uint16 | 1000 |
| 5 | EP16_XLog_Force | Extended logging: force trace | uint16 | 1000 |
| 6 | EP16_XLog_Error | Extended logging: error trace | uint16 | 1000 |
| 7 | EP16_XLog_TorqueLimit | Extended logging: torque limit trace | uint16 | 1000 |
| 1 | EP32_Data_CtrlIncrements | Position controller input data | uint32 | 500 |
| 2 | EP32_Data_Position | Resolver position data | uint32 | 500 |

---
### Notes
- Data Type: all registers use `uint16` unless noted; EP32 data are logically `uint32` and may be stored as two words.
- Volatility: 0–63 are saved to EPROM (NV), 64–95 are runtime (volatile), 96–159 and 256+ are read‑only.
- Communication: CAN (internal/external), RS‑232 (SCCI), SPI (DAC/TRM/optical sensors).
- Categories: Device Configuration & Calibration; Safety & Monitoring Control; Runtime Control Parameters; Debug Parameters & Service; Security & Locking; Device Status; Sensors/Telemetry & Diagnostics; NV Data Management & Boot.

Generated from sources: Platform/DeviceObjectDictionary.h, Platform/DataTable.h, Platform/DataTable.c, Platform/Constraints.h, Platform/Constraints.c, Controller/Controller.c, Controller/Global.h, Sources/git_info.h. 
