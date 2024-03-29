﻿#ifndef __CANOPEN_TYPES_H
#define __CANOPEN_TYPES_H

// Include
#include "stdinc.h"
//
#include "ZwDSP.h"

// Types
//
// Pointers for IO functions
typedef Boolean (*CANopen_FUNC_SendMessage)(Int16U mBox, pCANMessage Data);
typedef void (*CANopen_FUNC_GetMessage)(Int16U mBox, pCANMessage Data);
typedef Int32U (*CANopen_FUNC_GetTimeStamp)();
typedef Boolean (*CANopen_FUNC_IsMessageReceived)(Int16U mBox, pBoolean pMessageLost);
typedef void (*CANopen_FUNC_ConfigMailbox)(Int16U mBox, Int32U MsgID, Boolean Dir, Int16U DataLen, Int32U Flags, Int16U TransmitPriority, Int32U LAM);
typedef void (*CANopen_FUNC_ClearMailbox)(Int16U mBox);
//
typedef void (*CANopen_FUNC_CallbackNetworkFail)();
//
// IO configuration
typedef struct __CANopen_IOConfig
{
	CANopen_FUNC_SendMessage IO_SendMessage;
	CANopen_FUNC_GetMessage IO_GetMessage;
	CANopen_FUNC_GetTimeStamp IO_GetTimeStamp;
	CANopen_FUNC_IsMessageReceived IO_IsMessageReceived;
	CANopen_FUNC_ConfigMailbox IO_ConfigMailbox;
	CANopen_FUNC_ClearMailbox IO_ClearMailbox;
} CANopen_IOConfig, *pCANopen_IOConfig;
//
// CANopen instance state
typedef struct __CANopen_Interface
{
	pCANopen_IOConfig IOConfig;
	CANopen_FUNC_CallbackNetworkFail NetworkFail;
} CANopen_Interface, *pCANopen_Interface;

#endif // __CANOPEN_TYPES_H
