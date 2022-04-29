// Header
#include "CANopen.h"

// Macro
//
#define MBOX_NMT			1
#define MBOX_TPDO1			2
#define MBOX_RPDO1			3
#define MBOX_SDO_TX			4
#define MBOX_SDO_RX			5
#define MBOX_TPDO2			6
//
#define MBOX_RX				TRUE
#define MBOX_TX				FALSE
//
#define CANOPEN_FC_NMT		0x0
#define CANOPEN_FC_TPDO1	0x3
#define CANOPEN_FC_RPDO1	0x4
#define CANOPEN_FC_TPDO2	0x5
#define CANOPEN_FC_SDO_TX	0xb
#define CANOPEN_FC_SDO_RX	0xc
//
#define CANOPEN_FC_OFFSET		(1 << 7)
#define CANOPEN_MSGID_OFFSET	(1ul << 18)
//
#define CANOPEN_SDO_TICK_TO		(CANOPEN_SDO_TIMEOUT * CANOPEN_BR / 1000)

// Variables
//
static Boolean CANopen_FaultFlag = FALSE;

// Forward functions
//
void CANopen_ComposeMessage(pCANMessage Message, pInt16U Data);
void CANopen_SdoRd_SendRequest(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex);
Boolean CANopen_SdoRd_GetResponse(pCANopen_Interface Interface, pInt16U Index, pInt16U SubIndex, pInt32U Value);
void CANopen_SdoWr_SendRequest(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, Int32U Value);
Boolean CANopen_SdoWr_GetResponse(pCANopen_Interface Interface, pInt16U Index, pInt16U SubIndex);

// Functions
//
void CANopen_Init(pCANopen_Interface Interface, pCANopen_IOConfig IOConfig, CANopen_FUNC_CallbackNetworkFail NetworkFail)
{
	// Save parameters
	Interface->IOConfig = IOConfig;
	Interface->NetworkFail = NetworkFail;

	// Setup messages
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_NMT,		(CANOPEN_FC_NMT * CANOPEN_FC_OFFSET + 0) * CANOPEN_MSGID_OFFSET,						MBOX_TX, 2, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_TPDO1,		(CANOPEN_FC_TPDO1 * CANOPEN_FC_OFFSET + CANOPEN_SLAVE_NODE_ID) * CANOPEN_MSGID_OFFSET,	MBOX_RX, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_RPDO1,		(CANOPEN_FC_RPDO1 * CANOPEN_FC_OFFSET + CANOPEN_SLAVE_NODE_ID) * CANOPEN_MSGID_OFFSET,	MBOX_TX, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_SDO_TX,	(CANOPEN_FC_SDO_TX * CANOPEN_FC_OFFSET + CANOPEN_SLAVE_NODE_ID) * CANOPEN_MSGID_OFFSET,	MBOX_RX, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_SDO_RX,	(CANOPEN_FC_SDO_RX * CANOPEN_FC_OFFSET + CANOPEN_SLAVE_NODE_ID) * CANOPEN_MSGID_OFFSET,	MBOX_TX, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
	Interface->IOConfig->IO_ConfigMailbox(MBOX_OFFSET + MBOX_TPDO2,		(CANOPEN_FC_TPDO2 * CANOPEN_FC_OFFSET + CANOPEN_SLAVE_NODE_ID) * CANOPEN_MSGID_OFFSET,	MBOX_RX, 8, ZW_CAN_MBProtected, ZW_CAN_NO_PRIORITY, ZW_CAN_STRONG_MATCH);
}
// ----------------------------------------

void CANopen_ClearFault()
{
	CANopen_FaultFlag = FALSE;
}
// ----------------------------------------

void CANopen_NMT_NodeStart(pCANopen_Interface Interface, Int16U NodeId)
{
	Int16U Data[8] = {0};
	CANMessage Message;

	// Organize data
	Data[0] = 0x1;
	Data[1] = NodeId;

	// Compose message
	CANopen_ComposeMessage(&Message, Data);

	// Write data to mailbox
	Interface->IOConfig->IO_SendMessage(MBOX_OFFSET + MBOX_NMT, &Message);
}
// ----------------------------------------

void CANopen_ComposeMessage(pCANMessage Message, pInt16U Data)
{
	Message->HIGH.WORD.WORD_0  = Data[0] << 8;
	Message->HIGH.WORD.WORD_0 |= Data[1] & 0xff;
	Message->HIGH.WORD.WORD_1  = Data[2] << 8;
	Message->HIGH.WORD.WORD_1 |= Data[3] & 0xff;
	Message->LOW.WORD.WORD_2   = Data[4] << 8;
	Message->LOW.WORD.WORD_2  |= Data[5] & 0xff;
	Message->LOW.WORD.WORD_3   = Data[6] << 8;
	Message->LOW.WORD.WORD_3  |= Data[7] & 0xff;
}
// ----------------------------------------

void CANopen_PdoWr(pCANopen_Interface Interface, pInt16U Data)
{
	if (!CANopen_FaultFlag)
	{
		CANMessage Message;

		// Compose message
		CANopen_ComposeMessage(&Message, Data);

		// Write data to mailbox
		Interface->IOConfig->IO_SendMessage(MBOX_OFFSET + MBOX_RPDO1, &Message);
	}
}
// ----------------------------------------

Boolean CANopen_PDOMonitor(pCANopen_Interface Interface, pInt32U ValueH, pInt32U ValueL)
{
	// Check response
	if (Interface->IOConfig->IO_IsMessageReceived(MBOX_OFFSET + MBOX_TPDO2, NULL))
	{
		CANMessage Message;

		// Read data
		Interface->IOConfig->IO_GetMessage(MBOX_OFFSET + MBOX_TPDO2, &Message);

		*ValueH = Message.HIGH.DWORD_0;
		*ValueL = Message.LOW.DWORD_1;

		return TRUE;
	}
	else
		return FALSE;
}
// ----------------------------------------

void CANopen_SdoRd_SendRequest(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex)
{
	CANMessage Message;
	Int16U Data[8] = {0};

	// Organize data
	Data[0] = 0x40;				// initiate upload request
	Data[1] = Index & 0xff;		// index - low octet
	Data[2] = Index >> 8;		// index - high octet
	Data[3] = SubIndex & 0xff;	// subindex

	// Compose message
	CANopen_ComposeMessage(&Message, Data);

	// Write data to mailbox
	Interface->IOConfig->IO_SendMessage(MBOX_OFFSET + MBOX_SDO_RX, &Message);
}
// ----------------------------------------

Boolean CANopen_SdoRd_GetResponse(pCANopen_Interface Interface, pInt16U Index, pInt16U SubIndex, pInt32U Value)
{
	Int16U ZeroByte, Counter, Data[4], i;
	CANMessage Message;

	// Read data
	Interface->IOConfig->IO_GetMessage(MBOX_OFFSET + MBOX_SDO_TX, &Message);
	ZeroByte = Message.HIGH.WORD.WORD_0 >> 8;

	// If initiate upload response incorrect
	if ((ZeroByte >> 5) != 2) return FALSE;

	// If transfer indicators incorrect
	if ((ZeroByte & 0x3) != 0x3) return FALSE;

	// Index
	*Index = Message.HIGH.WORD.WORD_0 & 0xff;
	*Index |= Message.HIGH.WORD.WORD_1 & 0xff00;

	// SubIndex
	*SubIndex = Message.HIGH.WORD.WORD_1 & 0xff;

	// Decompose data
	Data[0] = Message.LOW.WORD.WORD_2 >> 8;
	Data[1] = Message.LOW.WORD.WORD_2 & 0xff;
	Data[2] = Message.LOW.WORD.WORD_3 >> 8;
	Data[3] = Message.LOW.WORD.WORD_3 & 0xff;

	// Save to value
	Counter = 4 - ((ZeroByte >> 2) & 0x3);
	*Value = 0;
	for (i = 0; i < Counter; i++)
		*Value |= (Int32U)(Data[i]) << (i * 8);

	return TRUE;
}
// ----------------------------------------

void CANopen_SdoRd(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, pInt32U Value)
{
	if (!CANopen_FaultFlag)
	{
		Int32U TimeStamp_Start;
		Int16U IndexRd, SubIndexRd;

		// Check for possible ticker overflow
		while (INT32U_MAX - Interface->IOConfig->IO_GetTimeStamp() <= CANOPEN_SDO_TICK_TO) {}

		// Clear imput mailbox
		Interface->IOConfig->IO_ClearMailbox(MBOX_OFFSET + MBOX_SDO_TX);

		// Save timestamp
		TimeStamp_Start = Interface->IOConfig->IO_GetTimeStamp();

		// Send request
		CANopen_SdoRd_SendRequest(Interface, Index, SubIndex);

		// Wait for response
		while (!Interface->IOConfig->IO_IsMessageReceived(MBOX_OFFSET + MBOX_SDO_TX, NULL) &&\
				Interface->IOConfig->IO_GetTimeStamp() - TimeStamp_Start < CANOPEN_SDO_TICK_TO) {}

		// Exit if timeout occured
		if (Interface->IOConfig->IO_GetTimeStamp() - TimeStamp_Start > CANOPEN_SDO_TICK_TO)
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail(COE_ReadTimeout, Index, SubIndex, 0);
			return;
		}

		// Read response
		if (!CANopen_SdoRd_GetResponse(Interface, &IndexRd, &SubIndexRd, Value))
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail(COE_ReadBadResponse, Index, SubIndex, 0);
			return;
		}

		// If index or subindex doesn't match
		if (Index != IndexRd || SubIndex != SubIndexRd)
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail((Index != IndexRd) ? COE_ReadBadIndex : COE_ReadBadSubIndex, Index, SubIndex, *Value);
		}
	}
}
// ----------------------------------------

void CANopen_SdoWr_SendRequest(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, Int32U Value)
{
	CANMessage Message;
	Int16U Data[8] = {0};

	// Organize data
	Data[0] = 0x20;				// initiate download request
	Data[0] |= 0x3;				// configure flags
	Data[1] = Index & 0xff;		// index - low octet
	Data[2] = Index >> 8;		// index - high octet
	Data[3] = SubIndex & 0xff;	// subindex
	//
	Data[4] = Value & 0xff;
	Data[5] = (Value >> 8) & 0xff;
	Data[6] = (Value >> 16) & 0xff;
	Data[7] = (Value >> 24) & 0xff;

	// Compose message
	CANopen_ComposeMessage(&Message, Data);

	// Write data to mailbox
	Interface->IOConfig->IO_SendMessage(MBOX_OFFSET + MBOX_SDO_RX, &Message);
}
// ----------------------------------------

Boolean CANopen_SdoWr_GetResponse(pCANopen_Interface Interface, pInt16U Index, pInt16U SubIndex)
{
	Int16U ZeroByte;
	CANMessage Message;

	// Read data
	Interface->IOConfig->IO_GetMessage(MBOX_OFFSET + MBOX_SDO_TX, &Message);
	ZeroByte = Message.HIGH.WORD.WORD_0 >> 8;

	// If initiate download response incorrect
	if ((ZeroByte >> 5) != 3) return FALSE;

	// Index
	*Index = Message.HIGH.WORD.WORD_0 & 0xff;
	*Index |= Message.HIGH.WORD.WORD_1 & 0xff00;

	// SubIndex
	*SubIndex = Message.HIGH.WORD.WORD_1 & 0xff;

	return TRUE;
}
// ----------------------------------------

void CANopen_SdoWr(pCANopen_Interface Interface, Int16U Index, Int16U SubIndex, Int32U Value)
{
	if (!CANopen_FaultFlag)
	{
		Int32U TimeStamp_Start;
		Int16U IndexRd, SubIndexRd;

		// Check for possible ticker overflow
		while (INT32U_MAX - Interface->IOConfig->IO_GetTimeStamp() <= CANOPEN_SDO_TICK_TO) {}

		// Clear imput mailbox
		Interface->IOConfig->IO_ClearMailbox(MBOX_OFFSET + MBOX_SDO_TX);

		// Save timestamp
		TimeStamp_Start = Interface->IOConfig->IO_GetTimeStamp();

		// Send request
		CANopen_SdoWr_SendRequest(Interface, Index, SubIndex, Value);

		// Wait for response
		while (!Interface->IOConfig->IO_IsMessageReceived(MBOX_OFFSET + MBOX_SDO_TX, NULL) &&\
				Interface->IOConfig->IO_GetTimeStamp() - TimeStamp_Start < CANOPEN_SDO_TICK_TO) {}

		// Exit if timeout occured
		if (Interface->IOConfig->IO_GetTimeStamp() - TimeStamp_Start > CANOPEN_SDO_TICK_TO)
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail(COE_WriteTimeout, Index, SubIndex, 0);
			return;
		}

		// Read response
		if (!CANopen_SdoWr_GetResponse(Interface, &IndexRd, &SubIndexRd))
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail(COE_WriteBadResponse, Index, SubIndex, 0);
			return;
		}

		// If index or subindex doesn't match
		if (Index != IndexRd || SubIndex != SubIndexRd)
		{
			CANopen_FaultFlag = TRUE;
			Interface->NetworkFail((Index != IndexRd) ? COE_WriteBadIndex : COE_WriteBadSubIndex, Index, SubIndex, 0);
		}
	}
}
// ----------------------------------------

// No more.
