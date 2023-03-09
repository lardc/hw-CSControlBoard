#ifndef __ZBANALOG_INPUT_H
#define __ZBANALOG_INPUT_H

// Include
#include "stdinc.h"

// Functions
//
// Init strain gage analog input
void ZbAnanlogInput_Init();
// Get sampled data
Int16U ZbAnanlogInput_GetResult();
// Acquisition
void ZbAnanlogInput_StartAcquisition();
void ZbAnanlogInput_EnableAcq(Boolean Flag);

#endif // __ZBANALOG_INPUT_H
