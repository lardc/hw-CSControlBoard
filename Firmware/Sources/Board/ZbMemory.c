// ----------------------------------------
// Driver for MCU EEPROM
// ----------------------------------------

// Header
#include "ZbMemory.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"
#include "Flash.h"

// Definitions
#define FLASH_START_ADDR		0x3D8000

// Functions
//
void ZbMemory_WriteValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	DINT;
	Status = Flash_Erase(SECTORH, (FLASH_ST *)&FlashStatus);
	Status = Flash_Program((pInt16U)FLASH_START_ADDR, Buffer, BufferSize, (FLASH_ST *)&FlashStatus);
	EINT;
}
// ----------------------------------------

void ZbMemory_ReadValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	Int16U i;
	pInt16U StartPointer = (pInt16U)FLASH_START_ADDR;
	for(i = 0; i < BufferSize; i++)
		Buffer[i] = *(StartPointer + i);
}
// ----------------------------------------
