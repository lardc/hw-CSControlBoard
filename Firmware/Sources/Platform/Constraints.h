// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "Global.h"
#include "StepperMotor.h"

// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;

// Position parameters
#define POS_MAX						180
#define SPEED_MAX					(Int16U)SM_MAX_SPEED/1000

// Device parameters
#define DEV_HEIGHT_MIN				0		// (in mm)
#define DEV_HEIGHT_MAX				70		// (in mm)
#define DEV_HEIGHT_DEF				0		// (in mm)

// Device offset
#define POS_DEV_OFFS_MIN			0		// (in mm)
#define POS_DEV_OFFS_MAX			50		// (in mm)
#define POS_DEV_OFFS_DEF			20		// (in mm)

// Temperature
#define TRM_TEMP_MIN				0		// in C x10
#define TRM_TEMP_MAX				2000	// in C x10
#define TRM_TEMP_DEF				0		// in C x10

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
