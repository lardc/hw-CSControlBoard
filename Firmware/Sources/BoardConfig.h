#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Include
#include <ZwBase.h>

// Program build mode
//
#define BOOT_FROM_FLASH						// normal mode
#define RAM_CACHE_SPI_ABCD					// cache SPI-A(BCD) functions

// Board options
#define DSP28_2809							// on-board CPU
#define OSC_FRQ				(20MHz)			// on-board oscillator
#define CPU_FRQ_MHZ			100				// CPU frequency = 100MHz
#define CPU_FRQ				(CPU_FRQ_MHZ * 1000000L) 
#define SYS_HSP_FREQ		(CPU_FRQ / 2) 	// High-speed bus freq = 50MHz
#define SYS_LSP_FREQ		(CPU_FRQ / 4) 	// Low-speed bus freq = 25MHz
//
#define ZW_PWM_DUTY_BASE	5000

// Peripheral options
#define HWUSE_SPI_A
#define HWUSE_SPI_B
#define HWUSE_SCI_A
#define HWUSE_SCI_B

// Analog input
#define ADC_INA0			0x00
#define ADC_SNUM			16				// Number of samples (1 to 16)

// IO placement
#define SPI_A_QSEL		    GPAQSEL2
#define SPI_A_MUX			GPAMUX2
#define SPI_A_SIMO			GPIO16
#define SPI_A_SOMI			GPIO17
#define SPI_A_CLK			GPIO18
#define SPI_A_CS			GPIO19
//
#define SPI_B_QSEL		    GPAQSEL2
#define SPI_B_MUX			GPAMUX2
#define SPI_B_SIMO			GPIO24
#define SPI_B_SOMI			GPIO25
#define SPI_B_CLK			GPIO26
#define SPI_B_CS			GPIO27
//
#define SCI_A_QSEL			GPAQSEL2
#define SCI_A_MUX			GPAMUX2
#define SCI_A_RX			GPIO28
#define SCI_A_TX			GPIO29
#define SCI_A_MUX_SELECTOR	1
//
#define SCI_B_QSEL			GPAQSEL1
#define SCI_B_MUX			GPAMUX1
#define SCI_B_RX			GPIO11
#define SCI_B_TX			GPIO9
#define SCI_B_MUX_SELECTOR	2
//
#define PIN_WD_RST			32
#define PIN_M1M2			2
#define PIN_M3M4			3
#define PIN_SEN1			8
#define PIN_SEN2			10
#define PIN_SEN3			6
#define PIN_SAFETY			22
#define PIN_SAFETY_HOLD		1
#define PIN_FAN				0
#define PIN_POWER_SWITCH	33
#define PIN_LED				12
#define PIN_SPIMUX_A		14
#define PIN_SPIMUX_B		15
#define PIN_SPIMUX_C		23
#define PIN_RS485_CTRL		24
#define PIN_AOUT_LDAC		13

#endif // __BOARD_CONFIG_H
