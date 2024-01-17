#ifndef __TEMPERATUREFEEDBACK_H
#define __TEMPERATUREFEEDBACK_H

// Include
#include "stdinc.h"
#include "IQmathUtils.h"

// Functions
//
// Basic read functions
_iq TempFb_GetTemperatureCH1();
_iq TempFb_GetTemperatureCH2();
// Update feedback information
void TempFb_UpdateTemperatureFeedback(Int64U TimeCounter);

#endif // __TEMPERATUREFEEDBACK_H
