// Header
//
#include "ZbCSAdapter.h"

// Include
//
#include "DS18B20.h"

Boolean CSAdapter_ReadID(pInt16U Data)
{
	return DS18B20_ReadReg(Data);
}
//------------------------------------------

Boolean CSAdapter_WriteID(pInt16U Data)
{
	return DS18B20_WriteReg(Data);
}
//------------------------------------------




