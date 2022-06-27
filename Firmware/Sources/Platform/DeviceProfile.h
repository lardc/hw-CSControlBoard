// -----------------------------------------
// Device profile
// ----------------------------------------

#ifndef __DEV_PROFILE_H
#define __DEV_PROFILE_H

// Include
#include "BCCISlave.h"
#include "CANopen.h"
#include "OWENProtocol.h"
#include "SCCISlave.h"
#include "stdinc.h"
//

// Variables
extern CANopen_Interface DEVICE_CANopen_Interface;

// Functions
//
// Initialize device profile engine
void DEVPROFILE_Init(xCCI_FUNC_CallbackAction SpecializedDispatch, volatile Boolean *MaskChanges);
// Initialize endpoint service 16 bit
void DEVPROFILE_InitEPService16(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas);
// Initialize endpoint service 32 bit
void DEVPROFILE_InitEPService32(pInt16U Indexes, pInt16U Sizes, pInt16U *Counters, pInt16U *Datas);
// Process user interface requests
void DEVPROFILE_ProcessRequests();
// Reset EP counters
void DEVPROFILE_ResetEPReadState();
// Reset user control (WR) section of data table
void DEVPROFILE_ResetControlSection();
// Fill non-volatile area with default values
void DEVPROFILE_FillNVPartDefault();
// Reset scopes 16 bit
void DEVPROFILE_ResetScopes16(Int16U ResetPosition);
// Reset scopes 32 bit
void DEVPROFILE_ResetScopes32(Int16U ResetPosition);
// Notify that CANa system fault occurs
void DEVPROFILE_NotifyCANaFault(ZwCAN_SysFlags Flag);
// Notify that CANb system fault occurs
void DEVPROFILE_NotifyCANbFault(ZwCAN_SysFlags Flag);
// Update diagnostic registers
void DEVPROFILE_UpdateCANDiagStatus();

// Read 32-bit value from data table
Int32U DEVPROFILE_ReadValue32(pInt16U pTable, Int16U Index);
// Write 32-bit value to data table
void DEVPROFILE_WriteValue32(pInt16U pTable, Int16U Index, Int32U Data);

#endif // __DEV_PROFILE_H
