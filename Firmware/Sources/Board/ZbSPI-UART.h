#ifndef __ZBSPIUART_H
#define __ZBSPIUART_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"

// Functions
//
void ZbSUe_Init();
void ZbSUe_SendData(pInt16U Buffer, Int16U BufferSize);
Int16S ZbSUe_ReadData(pInt16U Buffer, Int16U BufferSize);
void ZbSUe_UpdateTimeCounter(Int64U Counter);

#endif // __ZBSPIUART_H
