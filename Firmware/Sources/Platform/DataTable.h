﻿// -----------------------------------------
// Device data table
// ----------------------------------------

#ifndef __DATA_TABLE_H
#define __DATA_TABLE_H

// Include
#include "stdinc.h"


// Constants
//
#define DATA_TABLE_SIZE         300
#define DATA_TABLE_NV_SIZE      64
//
#define DATA_TABLE_NV_START     0
#define DATA_TABLE_WR_START     64
#define DATA_TABLE_WP_START     96

/*
 * DATA TABLE START 				------------- 0
 * 			[NON-VOLATILE AREA]
 *		END OF NON-VOLATILE AREA	------------- 63
 * 		START OF READ/WRITE AREA	------------- 64
 * 			[VOLATILE AREA]
 *		END OF READ/WRITE AREA		------------- 95
 * 		START OF READ-ONLY AREA		------------- 96
 * 			[VOLATILE R-O AREA]
 *		END OF READ-ONLY AREA		------------- 159
 * DATA TABLE END 					------------- [159]
 */


// Types
//
typedef void (*FUNC_SetDefaultValues)();
typedef void (*FUNC_EPROM_WriteValues)(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize);
typedef void (*FUNC_EPROM_ReadValues)(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize);
//
typedef struct __EPROMServiceConfig
{
	FUNC_EPROM_WriteValues WriteService;
	FUNC_EPROM_ReadValues ReadService;
} EPROMServiceConfig, *pERPOMServiceConfig;


// Variables
//
extern volatile Int16U DataTable[DATA_TABLE_SIZE];


// Functions
//
// Initialize data table
void DT_Init(EPROMServiceConfig EPROMService, Boolean NoRestore);
// Restore part of data table from non-volatile EPROM memory
void DT_RestoreNVPartFromEPROM();
// Save part of data table to non-volatile EPROM memory
void DT_SaveNVPartToEPROM();
// Reset non-volatile part of data table
void DT_ResetNVPart(FUNC_SetDefaultValues SetFunc);
// Reset volatile read-write part of data table
void DT_ResetWRPart(FUNC_SetDefaultValues SetFunc);
// Save firmware information to data table
void DT_SaveFirmwareInfo(Int16U SlaveNID, Int16U MasterNID);

#endif // __DATA_TABLE_H
