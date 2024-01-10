#ifndef __CANOPEN_PARAMS_H
#define __CANOPEN_PARAMS_H

// Include
#include "SysConfig.h"

// Constants
//
// Node ID for motor controller
#define CANOPEN_SLAVE_NODE_ID			2
// Mailbox number offset
#define MBOX_OFFSET						20
// SDO timeout (in ms)
#define CANOPEN_SDO_TIMEOUT				30
// Bus baudrate
#define CANOPEN_BR						CANB_BR

#endif // __CANOPEN_PARAMS_H
