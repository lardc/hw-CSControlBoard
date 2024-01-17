#ifndef __ZBDAC_H
#define __ZBDAC_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"

// Functions
//
// Write to DAC (A-channel)
void ZbDAC_WriteA(Int16U Data);
// Write to DAC (B-channel)
void ZbDAC_WriteB(Int16U Data);

#endif // __ZBDAC_H
