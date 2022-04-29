#ifndef __CANOPEN_H
#define __CANOPEN_H

// Include
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "CANopenParams.h"
#include "CANopenTypes.h"

// Functions
//
// Init interface instance
void CANopen_Init(pCANopen_Interface Interface, pCANopen_IOConfig IOConfig, CANopen_FUNC_CallbackNetworkFail NetworkFail);
// Clear CANopen fault
void CANopen_ClearFault();
// Write PDO message to slave
void CANopen_PdoWr(pCANopen_Interface Interface, pInt16U Data);
// Read SDO data
void CANopen_SdoRd(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, pInt32U Value);
// Write SDO data
void CANopen_SdoWr(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, Int32U Value);
// Send start node NMT command
void CANopen_NMT_NodeStart(pCANopen_Interface Interface, Int16U NodeId);

Boolean CANopen_PDOMonitor(pCANopen_Interface Interface, pInt32U ValueH, pInt32U ValueL);

#endif // __CANOPEN_H
