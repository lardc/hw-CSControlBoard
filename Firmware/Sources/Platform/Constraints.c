// -----------------------------------------
// Global definitions
// ----------------------------------------

// Header
#include "Constraints.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"

#define NO		0	// equal to FALSE
#define YES		1	// equal to TRUE

// Constants
//
const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE] =
                                      {
											   {0, 4095, 0},															// 0
											   {0, 0, 0},																// 1
											   {0, 0, 0},																// 2
											   {0, 0, 0},																// 3
											   {0, 0, 0},																// 4
											   {0, 0, 0},																// 5
											   {0, 0, 0},																// 6
											   {0, 0, 0},																// 7
											   {0, 0, 0},																// 8
											   {0, 0, 0},																// 9
											   {0, INT16U_MAX, FORCE_K_N_DEF},											// 10
											   {1, X_D_MAX, X_D_DEF},													// 11
											   {0, INT16U_MAX, 0},														// 12
											   {0, INT16U_MAX, X_D_DEF2},												// 13
											   {0, INT16U_MAX, 0},														// 14
											   {HOMING_REF_SPEED_MIN, HOMING_REF_SPEED_MAX, HOMING_REF_SPEED_DEF},		// 15
											   {HOMING_SEARCH_SPEED_MIN, HOMING_SEARCH_SPEED_MAX, HOMING_SEARCH_SPEED_DEF},	// 16
											   {HOMING_OFFSET_MIN, HOMING_OFFSET_MAX, HOMING_OFFSET_DEF},				// 17
											   {HOMING_TORQUE_MIN, HOMING_TORQUE_MAX, HOMING_TORQUE_DEF},				// 18
											   {POSITION_SPEED_MIN, POSITION_SPEED_MAX, POSITION_SPEED_DEF},			// 19
											   {POSITION_TORQUE_MIN, POSITION_TORQUE_MAX, POSITION_TORQUE_DEF},			// 20
											   {CLAMP_SPEED_MIN, CLAMP_SPEED_MAX, CLAMP_SPEED_DEF},						// 21
											   {CLAMP_TORQUE_LIMIT_MIN, CLAMP_TORQUE_LIMIT_MAX, CLAMP_TORQUE_LIMIT_DEF},// 22
											   {CLAMP_TIMEOUT_MIN, CLAMP_TIMEOUT_MAX, CLAMP_TIMEOUT_DEF},				// 23
											   {CLAMP_ERR_ZONE_MIN, CLAMP_ERR_ZONE_MAX, CLAMP_ERR_ZONE_DEF},			// 24
											   {FORCE_VAL_MIN, FORCE_VAL_MAX, FORCE_VAL_DEF},							// 25
											   {AUTO_RLS_TIMEOUT_MIN, AUTO_RLS_TIMEOUT_MAX, AUTO_RLS_TIMEOUT_DEF},		// 26
											   {POS_RLS_OFFS_MIN, POS_RLS_OFFS_MAX, POS_RLS_OFFS_DEF},					// 27
											   {POS_DEV_OFFS_MIN, POS_DEV_OFFS_MAX, POS_DEV_OFFS_DEF},					// 28
											   {0, INT16U_MAX, FORCE_Kp_N_DEF},											// 29
											   {1, X_D_MAX, X_D_DEF},													// 30
											   {0, INT16U_MAX, FORCE_Kp_POST_N_DEF},									// 31
											   {1, X_D_MAX, X_D_DEF},													// 32
											   {NO, YES, YES},															// 33
											   {0, 0, 0},																// 34
											   {0, 0, 0},																// 35
											   {0, 0, 0},																// 36
											   {0, 0, 0},																// 37
											   {0, 0, 0},																// 38
											   {0, 0, 0},																// 39
											   {0, 0, 0},																// 40
											   {0, 0, 0},																// 41
											   {0, 0, 0},																// 42
											   {0, 0, 0},																// 43
											   {0, 0, 0},																// 44
											   {0, 0, 0},																// 45
											   {NO, YES, YES},															// 46
											   {0, 0, 0},																// 47
											   {0, 0, 0},																// 48
											   {0, 0, 0},																// 49
											   {0, 0, 0},																// 50
											   {FORCE_VAL_MIN, FORCE_VAL_MAX, FORCE_VAL_DEF},							// 51
											   {NO, YES, YES},															// 52
											   {NO, YES, YES},															// 53
											   {NO, YES, YES},															// 54
											   {1, X_D_MAX, X_D_DEF},													// 55
											   {1, X_D_MAX, X_D_DEF},													// 56
											   {0, POS_MAX, POS_MAX},													// 57
											   {0, POS_MAX, POS_MAX},													// 58
											   {0, INT16U_MAX, 0},														// 59
											   {NO, YES, YES},															// 60
											   {0, 0, 0},																// 61
											   {0, 0, 0},																// 62
                                    		   {INT16U_MAX, 0, 0}														// 63
                                      };

const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START] =
                                      {
											   {0, POS_MAX, 0},															// 64
											   {0, 0, 0},																// 65
											   {0, 0, 0},																// 66
											   {0, 0, 0},																// 67
											   {0, 0, 0},																// 68
											   {0, 0, 0},																// 69
											   {FORCE_VAL_MIN, FORCE_VAL_MAX, FORCE_VAL_DEF},							// 70
											   {DEV_HEIGHT_MIN, DEV_HEIGHT_MAX, DEV_HEIGHT_DEF},						// 71
											   {TRM_TEMP_MIN, TRM_TEMP_MAX, TRM_TEMP_DEF},								// 72
											   {NO, YES, NO},															// 73
											   {0, 0, 0},																// 74
											   {0, 0, 0},																// 75
											   {0, 0, 0},																// 76
											   {0, 0, 0},																// 77
											   {0, 0, 0},																// 78
											   {0, 0, 0},																// 79
											   {0, INT16U_MAX, 0},														// 80
											   {0, INT16U_MAX, 0},														// 81
											   {1, 2, 1},																// 82
											   {0, INT16U_MAX, 0},														// 83
											   {0, 255, 0},																// 84
											   {NO, YES, NO},															// 85
											   {0, INT16U_MAX, 0},														// 86
											   {0, INT16U_MAX, 0},														// 87
											   {0, 0, 0},																// 88
											   {0, 0, 0},																// 89
											   {0, 0, 0},																// 90
											   {0, 0, 0},																// 91
											   {0, 0, 0},																// 92
											   {0, 0, 0},																// 93
											   {0, 0, 0},																// 94
                                    		   {INT16U_MAX, 0, 0}														// 95
                                      };

// No more
