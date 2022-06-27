#ifndef __ZBSPIUART_H
#define __ZBSPIUART_H


// Include
#include "stdinc.h"
#include "ZwDSP.h"

// Functions
//
void ZbSU_SendData(pInt16U Buffer, Int16U BufferSize);
Int16S ZbSU_ReadData(pInt16U Buffer, Int16U BufferSize);
void ZbSU_UpdateTimeCounter(Int64U Counter);

#endif // __ZBSPIUART_H
