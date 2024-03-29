﻿// -----------------------------------------
// System parameters
// -----------------------------------------

#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

// Include
#include <ZwBase.h>
#include <BoardConfig.h>

// CPU & System
//--------------------------------------------------------
#define CPU_PLL				10          // OSCCLK * PLL div 2 = CPUCLK: 20 * 10 / 2 = 100
#define CPU_CLKINDIV		0           // "div 2" in previous equation
#define SYS_HISPCP       	0x01   		// SYSCLKOUT / 2
#define SYS_LOSPCP       	0x02    	// SYSCLKOUT / 4
//--------------------------------------------------------

// Power control
//--------------------------------------------------------
#define SYS_PUMOD			ZW_POWER_SPIA_CLK | ZW_POWER_SPIB_CLK |\
							ZW_POWER_CANA_CLK | ZW_POWER_CANB_CLK |\
							ZW_POWER_SCIA_CLK | ZW_POWER_SCIB_CLK |\
							ZW_POWER_ADC_CLK | ZW_POWER_PWM1_CLK

#define SYS_WD_PRESCALER	0x07
//--------------------------------------------------------

// Boot-loader
//--------------------------------------------------------
#define BOOT_LOADER_REQUEST	0xABCD
//--------------------------------------------------------

// GPIO
//--------------------------------------------------------
#define GPIO_TSAMPLE		0xFF		// T[sample_A] = (1/ 100MHz) * (2 * 255) = 5.1 uS
#define GPIO_NSAMPLE		6			// 6 samples: T = 5.1uS * 6 = 31 uS
//--------------------------------------------------------

// Flash
//--------------------------------------------------------
#define FLASH_FWAIT			3
#define FLASH_OTPWAIT		5
//--------------------------------------------------------

// TIMERs
//--------------------------------------------------------
#define CS_REALTIME_FREQ	50000L		// in Hz
#define CS_MONITORING_FREQ	10L			// in Hz

#define TIMER0_PERIOD		(1000000L / CS_REALTIME_FREQ)
#define TIMER2_PERIOD		(1000000L / CS_MONITORING_FREQ)

#define DBG_FREQ			2			// 2 Hz
#define DBG_COUNTER_PERIOD	(CS_MONITORING_FREQ / (DBG_FREQ * 2))
//--------------------------------------------------------

// ADC
//--------------------------------------------------------
#define ADC_PRESCALER		0			// HSPCLK / (1 * 1) = 50 MHz
#define ADC_CD2				TRUE		// Div ADC core / 2 = 25 MHz
#define ADC_SH				2			// S/H sample window = 2 => 6.25 MSPS
//--------------------------------------------------------

// Temperature feedback
//--------------------------------------------------------
#define TEMPFB_CONV_TIME	100			// in ms
#define TEMPFB_CONV_TICKS	((CS_MONITORING_FREQ * TEMPFB_CONV_TIME) / 1000)
//--------------------------------------------------------

// TRM
//--------------------------------------------------------
#define TRM_CH1_ADDR		1
#define TRM_CH2_ADDR		2
#define TRM_TEMP_THR		500			// in C x10
#define TRM_ROOM_TEMP		250			// in C x10
#define TRM_READ_TIMEOUT	100			// in ms
#define TRM_TIMEOUT_TICKS	((CS_MONITORING_FREQ * TRM_READ_TIMEOUT) / 1000)
//--------------------------------------------------------

// SPI mux
//--------------------------------------------------------
#define SPIMUX_AOUT			0
#define SPIMUX_SU			1
#define SPIMUX_OPTO1		2
#define SPIMUX_OPTO2		3
#define SPIMUX_EPROM		4
//--------------------------------------------------------

// SPI-A
//--------------------------------------------------------
#define SPIA_BAUDRATE		5000000L	// SPI clock (in Hz)
#define SPIA_PLR			FALSE		// CLK low in idle state
#define SPIA_PHASE			TRUE
//--------------------------------------------------------

// SPI-B
//--------------------------------------------------------
#define SPIB_BAUDRATE		500000L		// SPI clock (in Hz)
#define SPIB_PLR			FALSE		// CLK low in idle state
#define SPIB_PHASE			FALSE
//--------------------------------------------------------

// CAN-A
//--------------------------------------------------------
#define CANA_BR				1000000L
#define CANA_BRP			9
#define CANA_TSEG1			6
#define CANA_TSEG2			1
#define CANA_SJW			1
//--------------------------------------------------------

// CAN-B
//--------------------------------------------------------
#define CANB_BR				1000000L
#define CANB_BRP			9
#define CANB_TSEG1			6
#define CANB_TSEG2			1
#define CANB_SJW			1
//--------------------------------------------------------

// SCI-A
//--------------------------------------------------------
#define SCIA_BR				115200L
#define SCIA_DB				8
#define SCIA_SB				FALSE
#define SCIA_PARITY			ZW_PAR_NONE
//--------------------------------------------------------

// SCI-B
//--------------------------------------------------------
#define SCIB_BR				115200L
#define SCIB_DB				8
#define SCIB_SB				FALSE
#define SCIB_PARITY			ZW_PAR_NONE
//--------------------------------------------------------

// Fan
//--------------------------------------------------------
#define FAN_TIMEOUT			30000			// in ms
#define FAN_TIMEOUT_TCK		((CS_MONITORING_FREQ * FAN_TIMEOUT) / 1000)
//--------------------------------------------------------

// Clamp
//--------------------------------------------------------
#define DELAY_CHANGE_DEV_STATE		200		// in ms
#define DELAY_OP_COMPLETE			5		// in ms
//--------------------------------------------------------

#endif // __SYSCONFIG_H
