// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] =
{
	{"Homing duration",					DT_Int32U, 1},
	{"ClampingDuration",				DT_Int32U, 1},
	{"ReleaseDuration",					DT_Int32U, 1},
};    
Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);
