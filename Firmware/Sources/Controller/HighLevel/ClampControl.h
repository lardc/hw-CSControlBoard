#ifndef __CLAMPCTRL_H
#define __CLAMPCTRL_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"
#include "Controller.h"

// Functions
//
void CLAMPCTRL_StartClamping();
Boolean CLAMPCTRL_IsClampingDone();
void CLAMPCTRL_ClampingUpdateRequest();
void CLAMPCTRL_StartClampingRelease(Boolean PositionMode);
Boolean CLAMPCTRL_IsClampingReleaseDone();
void CLAMPCTRL_CacheVariables();
void CLAMPCTRL_XLog(DeviceState State);

#endif // __CLAMPCTRL_H
