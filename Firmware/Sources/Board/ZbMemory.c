// -----------------------------------------
// Driver for EEPROM & FRAM via SPI
// ----------------------------------------

// Header
#include "ZbMemory.h"
//
#include "SysConfig.h"
#include "ZbGPIO.h"

// Constants
//
#define DATA_BUFFER_SIZE		16
#define EPROM_DATA_SEGMENT		4		// 4 * 2 + (3) < 16 - SPI FIFO max depth
#define EPROM_WRITE_DELAY_US	5000
//
// Commands
#define MEM_WRITE				0x02
#define MEM_READ				0x03
#define EPROM_WREN				0x06

// Variables
//
static Int16U DataBuffer[DATA_BUFFER_SIZE];

// Forward functions
//
static void ZbMemory_EnableWriteEPROM();

// Functions
//
void ZbMemory_WriteValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	Int16U i, j, segCount;
	ZwMemory_PrepareSPIForEPROM();
	
	// Calculate segment count: only 16 SRAM bytes can be written per one transaction (FIFO limit)
	segCount = (BufferSize / EPROM_DATA_SEGMENT) + ((BufferSize % EPROM_DATA_SEGMENT) ? 1 : 0); 
		
	// Write segments
	for(j = 0; j < segCount; ++j)
	{
		Int16U dataSize;
		
		// Calculate address for next segment
		Int16U currentEPROMAddress = EPROMAddress + j * EPROM_DATA_SEGMENT * 2;

		// Enable writing for next operation
		ZbMemory_EnableWriteEPROM();

		// Write command ID
		DataBuffer[0] = MEM_WRITE;
		// Memory address
		DataBuffer[1] = (currentEPROMAddress >> 8);
		DataBuffer[2] = currentEPROMAddress & 0x00FF;
		// Write data
		for(i = 0; i < MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT); ++i)
		{
			DataBuffer[3 + i * 2] = Buffer[j * EPROM_DATA_SEGMENT + i] >> 8;
			DataBuffer[3 + i * 2 + 1] = Buffer[j * EPROM_DATA_SEGMENT + i] & 0x00FF;
		}
	
		// Do SPI communication
		dataSize = 3 + MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT) * 2;
		ZwSPIa_Send(DataBuffer, dataSize, 8, STTNormal);

		DELAY_US(EPROM_WRITE_DELAY_US);
	}

	// Clear garbage
	ZwSPIa_ResetRXFIFO();
}
// ----------------------------------------

void ZbMemory_ReadValuesEPROM(Int16U EPROMAddress, pInt16U Buffer, Int16U BufferSize)
{
	Int16U i, j, segCount, dataSize;
	ZwMemory_PrepareSPIForEPROM();

	// Calculate segment count: only 16 FRAM bytes can be read per one transaction (FIFO limit)
	segCount = (BufferSize / EPROM_DATA_SEGMENT) + ((BufferSize % EPROM_DATA_SEGMENT) ? 1 : 0); 
		
	// Read segments
	for(j = 0; j < segCount; ++j)
	{
		// Calculate address for next segment
		Int16U currentEPROMAddress = EPROMAddress + j * EPROM_DATA_SEGMENT * 2;
		
		// Write command ID
		DataBuffer[0] = MEM_READ;
		// Memory address
		DataBuffer[1] = (currentEPROMAddress >> 8);
		DataBuffer[2] = currentEPROMAddress & 0x00FF;
		
		// Do SPI communication
		dataSize = 3 + MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT) * 2;
		ZwSPIa_BeginReceive(DataBuffer, dataSize, 8, STTNormal);
		while(ZwSPIa_GetWordsToReceive() < dataSize)
			DELAY_US(1);
		ZwSPIa_EndReceive(DataBuffer, dataSize);
		
		// Copy data
		for(i = 0; i < MIN(BufferSize - j * EPROM_DATA_SEGMENT, EPROM_DATA_SEGMENT); ++i)
		{
			Int16U result;
			
			// Get data from bytes
			result = (DataBuffer[3 + i * 2] & 0x00FF) << 8;
			result |= DataBuffer[3 + i * 2 + 1] & 0x00FF;
			
			Buffer[j * EPROM_DATA_SEGMENT + i] = result;
		}
	}
}
// ----------------------------------------

static void ZbMemory_EnableWriteEPROM()
{
	// Write @Enable@ command
	DataBuffer[0] = EPROM_WREN;
	// Do SPI communication
	ZwSPIa_Send(DataBuffer, 1, 8, STTNormal);
}
// ----------------------------------------

void ZwMemory_PrepareSPIForEPROM()
{
	ZwSPIa_Init(TRUE, SPIA_BAUDRATE, 8, SPIA_PLR, SPIA_PHASE, 0, TRUE, FALSE);
	ZbGPIO_CSMux(SPIMUX_EPROM);
}
// ----------------------------------------

// No more.
