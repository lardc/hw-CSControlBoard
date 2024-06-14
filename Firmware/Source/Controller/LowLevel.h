#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"


// Functions
//
void LL_ToggleBoardLED();
void LL_SetStateIndCSM(bool State);
void LL_SetStateIndADPTR(bool State);
void LL_SetStatePneumTOP(bool State);
void LL_SetStatePneumTOP(bool State);
void LL_SetStatePneumBOT(bool State);
void LL_SetStatePneumDUT(bool State);
void LL_SetStateSFOutput(bool State);
bool LL_GetStatePresenceSensorDUT1();
bool LL_GetStatePresenceSensorDUT2();
bool LL_GetStatePresenceSensorDUT3();
bool LL_GetStatePresenceSensorDUT4();
bool LL_GetStateLimitSwitchTopAdapter();
bool LL_GetStateLimitSwitchBotAdapter();
float LL_MeasureIDTop();
float LL_MeasureIDBot();
float LL_MeasurePressure();

#endif //__LOWLEVEL_H
