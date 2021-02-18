// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H


// Include
#include "stdinc.h"


// Constants
//
// Password to unlock non-volatile area for write
#define ENABLE_LOCKING				FALSE
#define UNLOCK_PWD_1				1
#define UNLOCK_PWD_2				1
#define UNLOCK_PWD_3				1
#define UNLOCK_PWD_4				1
//
#define DT_EPROM_ADDRESS			0
#define	SCCI_TIMEOUT_TICKS  		1000
#define SC_FILTER_T					100
//
#define EP_COUNT_16					7
#define EP_COUNT_32					2
#define VALUES_x_SIZE				500
#define VALUES_XLOG_x_SIZE			1000

// Debug modes
#define DBG_USE_SPI_UART			TRUE
#define DBG_USE_CLAMP_BRAKE			TRUE
#define DBG_USE_AIR_CTRL			FALSE

// Compatability patch
#undef PATCH_OLD_PC_SOFT
#define PATCH_LENZE_DI6_INVERT

#endif // __GLOBAL_H
