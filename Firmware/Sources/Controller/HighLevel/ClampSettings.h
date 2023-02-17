#ifndef __CLAMPSETTINGS_H
#define __CLAMPSETTINGS_H

// Definitions
//
#define NEUTON_TO_INCREMENT		_IQ(10.0f)			// Approximate force conversion factor (N) to (increment)
#define MM_TO_INCREMENT			100000l				// Approximate movement conversion factor (mm) to (increment)
#define STOP_DETECTION_TICKS	1000				// Detect clamping contact (in ticks)
#define FORCE_AVG_SAMPLES		40
//
#define CLAMP_LOWEST_TORQUE		_IQ(20.0f)			// in %
//
#define SERVICE_DELAY_MS		300
#define SERVICE_DELAY_TICKS		((SERVICE_DELAY_MS * CS_MONITORING_FREQ) / 1000)
//
#define CLAMP_DETECT_LIM		_IQ(3000.0f)		// Clamp detect low limit (N)

#endif // __CLAMPSETTINGS_H
