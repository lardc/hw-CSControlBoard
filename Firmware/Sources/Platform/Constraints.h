// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "ClampSettings.h"
#include "Global.h"

// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;

// Restricitions
//
#define FORCE_K_N_DEF				2751
#define FORCE_FINE_N_DEF			100
#define FORCE_Kp_N_DEF				40
#define FORCE_Kp_POST_N_DEF			10
//
#define X_D_DEF0					10
#define X_D_DEF1					100
#define X_D_DEF2					1000
#define X_D_DEF3					10000
//
#define X_D_DEF						X_D_DEF1
#define X_D_MAX						X_D_DEF3
//
// Homing ref speed
#define HOMING_REF_SPEED_MIN		100		// (in unit/s)
#define HOMING_REF_SPEED_MAX		5000	// (in unit/s)
#define HOMING_REF_SPEED_DEF		1000	// (in unit/s)

// Homing search speed
#define HOMING_SEARCH_SPEED_MIN		100		// (in unit/s)
#define HOMING_SEARCH_SPEED_MAX		5000	// (in unit/s)
#define HOMING_SEARCH_SPEED_DEF		100		// (in unit/s)

// Homing offset
#define HOMING_OFFSET_MIN			1		// (in units)
#define HOMING_OFFSET_MAX			10000	// (in units)
#define HOMING_OFFSET_DEF			2000	// (in units)

// Homing torque
#define HOMING_TORQUE_MIN			5		// (in %)
#define HOMING_TORQUE_MAX			50		// (in %)
#define HOMING_TORQUE_DEF			15		// (in %)

// Position speed
#define POSITION_SPEED_MIN			10		// (in %)
#define POSITION_SPEED_MAX			150		// (in %)
#define POSITION_SPEED_DEF			150		// (in %)

// Position torque
#define POSITION_TORQUE_MIN			10		// (in %)
#define POSITION_TORQUE_MAX			100		// (in %)
#define POSITION_TORQUE_DEF			50		// (in %)

// Clamp error
#define CLAMP_ERR_ZONE_MIN			1		// (in %)
#define CLAMP_ERR_ZONE_MAX			20		// (in %)
#define CLAMP_ERR_ZONE_DEF			5		// (in %)

// Clamp speed
#define CLAMP_SPEED_MIN				5		// (in %)
#define CLAMP_SPEED_MAX				50		// (in %)
#define CLAMP_SPEED_DEF				30		// (in %)

// Clamp torque limit
#define CLAMP_TORQUE_LIMIT_MIN		5		// (in %)
#define CLAMP_TORQUE_LIMIT_MAX		199		// (in %)
#define CLAMP_TORQUE_LIMIT_DEF		50		// (in %)

// Clamp timeout
#define CLAMP_TIMEOUT_MIN			1000	// (in ms)
#define CLAMP_TIMEOUT_MAX			60000	// (in ms)
#define CLAMP_TIMEOUT_DEF			2000	// (in ms)

// Clamp manual release timeout
#define AUTO_RLS_TIMEOUT_MIN		2000	// (in ms)
#define AUTO_RLS_TIMEOUT_MAX		10000	// (in ms)
#define AUTO_RLS_TIMEOUT_DEF		2000	// (in ms)

// Position settings
#define POS_RLS_OFFS_MIN			0		// (in mm)
#define POS_RLS_OFFS_MAX			50		// (in mm)
#define POS_RLS_OFFS_DEF			20		// (in mm)

// Position parameters
#define POS_MAX						INT16U_MAX

// Device parameters
#define DEV_HEIGHT_MIN				0		// (in mm)
#define DEV_HEIGHT_MAX				70		// (in mm)
#define DEV_HEIGHT_DEF				0		// (in mm)

// Device offset
#define POS_DEV_OFFS_MIN			0		// (in mm)
#define POS_DEV_OFFS_MAX			50		// (in mm)
#define POS_DEV_OFFS_DEF			20		// (in mm)

// Force settting
#define FORCE_VAL_MIN				50
#define FORCE_VAL_MAX				1100
#define FORCE_VAL_DEF				200

// Temperature
#define TRM_TEMP_MIN				0		// in C x10
#define TRM_TEMP_MAX				2000	// in C x10
#define TRM_TEMP_DEF				0		// in C x10
#define TEMP_READ_K_DEF				1000
#define TEMP_TO_DAC_K_DEF			1500


// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
