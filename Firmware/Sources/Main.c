// -----------------------------------------
// Program entry point
// ----------------------------------------

// Include
#include <stdinc.h>
//
#include "ZwDSP.h"
#include "ZbBoard.h"
#include "TRM101.h"
//
#include "SysConfig.h"
//
#include "Controller.h"
#include "Flash.h"

// FORWARD FUNCTIONS
// -----------------------------------------
Boolean InitializeCPU();
void InitializeTimers();
void InitializeADC();
void InitializeSPI();
void InitializeSCI();
void InitializeCAN();
void InitializeBoard();
void InitializeController(Boolean GoodClock);
// -----------------------------------------

// FORWARD ISRs
// -----------------------------------------
// CPU Timer 0 ISR
ISRCALL Timer0_ISR();
// CPU Timer 2 ISR
ISRCALL Timer2_ISR();
// ADC SEQ1 ISR
ISRCALL SEQ1_ISR();
// CANa Line 0 ISR
ISRCALL CAN0A_ISR();
// CANb Line 0 ISR
ISRCALL CAN0B_ISR();
// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR();
// -----------------------------------------

// FUNCTIONS
// -----------------------------------------
// Program main function
void main()
{
	Boolean clockInitResult;

	// Boot process
	clockInitResult = InitializeCPU();
	FLASH_Init();

	// Only if good clocking was established
	if(clockInitResult)
	{
		InitializeTimers();
		InitializeADC();
		InitializeSPI();
		InitializeCAN();
		InitializeBoard();
	}

	// Try initialize SCI in spite of result of clock initialization
	InitializeSCI();

	// Setup ISRs
	BEGIN_ISR_MAP
		ADD_ISR(TINT0, Timer0_ISR);
		ADD_ISR(TINT2, Timer2_ISR);
		ADD_ISR(ECAN0INTA, CAN0A_ISR);
		ADD_ISR(ECAN0INTB, CAN0B_ISR);
		ADD_ISR(SEQ1INT, SEQ1_ISR);
		ADD_ISR(ILLEGAL, IllegalInstruction_ISR);
	END_ISR_MAP

	// Only if good clocking was established
	if(clockInitResult)
	{
		// Enable interrupts
		EINT;
		ERTM;

		// Set watch-dog as WDRST
		ZwSystem_SelectDogFunc(FALSE);
		ZwSystem_EnableDog(SYS_WD_PRESCALER);

		// Start timers
		ZwTimer_StartT0();
		ZwTimer_StartT2();
	}

	// Initialize controller logic
	InitializeController(clockInitResult);

	// Low-priority services
	while(TRUE)
		CONTROL_Idle();
}
// -----------------------------------------

// Initialize and prepare DSP
Boolean InitializeCPU()
{
    Boolean clockInitResult;

	// Init clock and peripherals
    clockInitResult = ZwSystem_Init(CPU_PLL, CPU_CLKINDIV, SYS_LOSPCP, SYS_HISPCP, SYS_PUMOD);

    if(clockInitResult)
    {
		// Do default GPIO configuration
		ZwGPIO_Init(GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE);
		// Initialize PIE
		ZwPIE_Init();
		// Prepare PIE vectors
		ZwPIE_Prepare();
    }

	// Config flash
	ZW_FLASH_CODE_SHADOW;
	ZW_FLASH_MATH_SHADOW;
	ZW_FLASH_OPTIMIZE(FLASH_FWAIT, FLASH_OTPWAIT);

   	return clockInitResult;
}
// -----------------------------------------

// Initialize CPU timers
void InitializeTimers()
{
    ZwTimer_InitT0();
	ZwTimer_SetT0(TIMER0_PERIOD);
	ZwTimer_EnableInterruptsT0(TRUE);

    ZwTimer_InitT2();
 	ZwTimer_SetT2(TIMER2_PERIOD);
	ZwTimer_EnableInterruptsT2(TRUE);
}
// -----------------------------------------

void InitializeADC()
{
	// Initialize and prepare ADC
	ZwADC_Init(ADC_PRESCALER, ADC_CD2, ADC_SH);
	ZwADC_ConfigInterrupts(TRUE, FALSE);

	// Enable interrupts on peripheral and CPU levels
	ZwADC_EnableInterrupts(TRUE, FALSE);
	ZwADC_EnableInterruptsGlobal(TRUE);
}
// -----------------------------------------

// Initialize and prepare SCI modules
void InitializeSCI()
{
	// Initialize and prepare SCI modules
	ZwSCIa_Init(SCIA_BR, SCIA_DB, SCIA_PARITY, SCIA_SB, FALSE);
	ZwSCIa_InitFIFO(16, 0);
	ZwSCIa_EnableInterrupts(FALSE, FALSE);
	//
	ZwSCIb_Init(SCIB_BR, SCIB_DB, SCIB_PARITY, SCIB_SB, FALSE);
	ZwSCIb_InitFIFO(16, 0);
	ZwSCIb_EnableInterrupts(FALSE, FALSE);

	ZwSCI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeSPI()
{
	// Init SPI-A (DAC, SPI-UART, EPROM)
	ZwSPIa_Init(TRUE, SPIA_BAUDRATE, 16, SPIA_PLR, SPIA_PHASE, ZW_SPI_INIT_TX | ZW_SPI_INIT_RX | ZW_SPI_INIT_CS, FALSE, FALSE);
	ZwSPIa_InitFIFO(0, 0);
	ZwSPIa_ConfigInterrupts(FALSE, FALSE);
	ZwSPIa_EnableInterrupts(FALSE, FALSE);

	// Init SPI-B (Optical input)
	ZwSPIb_Init(TRUE, SPIB_BAUDRATE, 16, SPIB_PLR, SPIB_PHASE, ZW_SPI_INIT_RX | ZW_SPI_INIT_CS, FALSE, FALSE);
	ZwSPIb_InitFIFO(0, 0);
	ZwSPIb_ConfigInterrupts(FALSE, FALSE);
	ZwSPIb_EnableInterrupts(FALSE, FALSE);

	// Common (ABCD)
	ZwSPI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeCAN()
{
	// Init CAN
	ZwCANa_Init(CANA_BR, CANA_BRP, CANA_TSEG1, CANA_TSEG2, CANA_SJW);
	ZwCANb_Init(CANB_BR, CANB_BRP, CANB_TSEG1, CANB_TSEG2, CANB_SJW);

	// Register system handler
	ZwCANa_RegisterSysEventHandler(&CONTROL_NotifyCANaFault);
	ZwCANb_RegisterSysEventHandler(&CONTROL_NotifyCANbFault);

    // Allow interrupts for CANa (internal interface)
    ZwCANa_InitInterrupts(TRUE);
    ZwCANa_EnableInterrupts(TRUE);

    // Allow interrupts for CANb (CANopen interface)
	ZwCANb_InitInterrupts(TRUE);
	ZwCANb_EnableInterrupts(TRUE);
}
// -----------------------------------------

void InitializeBoard()
{
	// Init board GPIO
   	ZbGPIO_Init();
}
// -----------------------------------------

void InitializeController(Boolean GoodClock)
{
	// Init controllers and logic
	CONTROL_Init(!GoodClock);
}
// -----------------------------------------

// ISRs
// -----------------------------------------
#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(Timer0_ISR, "ramfuncs");
	#pragma CODE_SECTION(Timer2_ISR, "ramfuncs");
	#pragma CODE_SECTION(CAN0A_ISR, "ramfuncs");
	#pragma CODE_SECTION(CAN0B_ISR, "ramfuncs");
	#pragma CODE_SECTION(SEQ1_ISR, "ramfuncs");
	#pragma CODE_SECTION(IllegalInstruction_ISR, "ramfuncs");
#endif
//
#pragma INTERRUPT(Timer0_ISR, HPI);
#pragma INTERRUPT(SEQ1_ISR, HPI);

// Timer 0 ISR
ISRCALL Timer0_ISR(void)
{
	// Get analog input data
	ZbAnanlogInput_StartAcquisition();

	// allow other interrupts from group 1
	TIMER0_ISR_DONE;
}
// -----------------------------------------

// Timer 2 ISR
ISRCALL Timer2_ISR(void)
{
	static Int16U dbgCounter = 0;

	// Update tick counter
	++CONTROL_TimeCounter;
	// Update low-priority tasks
	CONTROL_UpdateLow();

	// Service watch-dogs
	if (CONTROL_BootLoaderRequest != BOOT_LOADER_REQUEST)
	{
		ZwSystem_ServiceDog();
		ZbWatchDog_Strobe();
	}

	++dbgCounter;
	if(dbgCounter == DBG_COUNTER_PERIOD)
	{
		ZbGPIO_ToggleLED();
		dbgCounter = 0;
	}

	// no PIE
	TIMER2_ISR_DONE;
}
// -----------------------------------------

// ADC SEQ1 ISR
ISRCALL SEQ1_ISR(void)
{
	// Handle interrupt
	ZwADC_ProcessInterruptSEQ1();
	// Dispatch results
	ZwADC_Dispatch1();

	// allow other interrupts from group 1
	ADC_ISR_DONE;
}
// -----------------------------------------

// Line 0 CANa ISR
ISRCALL CAN0A_ISR(void)
{
    // handle CAN system events
	ZwCANa_DispatchSysEvent();

	// allow other interrupts from group 9
	CAN_ISR_DONE;
}
// -----------------------------------------

// Line 0 CANb ISR
ISRCALL CAN0B_ISR(void)
{
    // handle CAN system events
	ZwCANb_DispatchSysEvent();

	// allow other interrupts from group 9
	CAN_ISR_DONE;
}
// -----------------------------------------

// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR(void)
{
	// Disable interrupts
	DINT;

	// Reset system using WD
	ZwSystem_ForceDog();
}
// -----------------------------------------

// No more.
