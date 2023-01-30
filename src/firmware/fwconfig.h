/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */
#ifndef _FWCONFIG_H_
#define _FWCONFIG_H_

#if defined(BBSHD)
	#define HAS_MOTOR_TEMP_SENSOR				1
#else
	#define HAS_MOTOR_TEMP_SENSOR				0
#endif


#if defined(BBSHD) || defined(BBS02)
	#define HAS_CONTROLLER_TEMP_SENSOR			1
#else
	#define HAS_CONTROLLER_TEMP_SENSOR			0
#endif


#if defined(TSDZ2)
	#define HAS_TORQUE_SENSOR					1
#else
	#define HAS_TORQUE_SENSOR					0
#endif


 // Applied to both motor and controller tmeperature sensor
#define MAX_TEMPERATURE							85

// Current ramp down starts at MAX_TEMPERATURE - 5.
#define MAX_TEMPERATURE_RAMP_DOWN_INTERVAL		5

// Maximum allowed motor current in percent of maximum configured current (A)
// to still apply when maximum temperature has been reached.
// Motor current is ramped down linearly until this value when approaching
// max temperature.
#define MAX_TEMPERATURE_LOW_CURRENT_PERCENT		20


#if defined(BBS02)
	#define MAX_CADENCE_RPM_X10					1500
#elif defined(BBSHD)
	// Measured on BBSHD at 48V
	#define MAX_CADENCE_RPM_X10					1680
#else
	#define MAX_CADENCE_RPM_X10					1200
#endif


// Current ramp down starts at LVC + (LVC * LVC_RAMP_DOWN_OFFSET_PERCENT / 100)
// Example:
// LVC is 42V
// 42 * 0.06 = 2.5V
// Ramp down starts at 42V + 2.5V
#define LVC_RAMP_DOWN_OFFSET_PERCENT			6

// Maximum allowed motor current in percent of maximum configured current (A)
// to still apply when LVC has been reached.
// Motor current is ramped down linearly until this value when approacing LVC.
#define LVC_LOW_CURRENT_PERCENT					20


// Size of speed limit ramp down interval.
// If max speed is 50 and this is set to 3 then the
// target current will start ramping down when passing 47
// and be at 50% of assist target current when reaching 50.
#define SPEED_LIMIT_RAMP_DOWN_INTERVAL_KPH		3

// Current ramp down (e.g. when releasing throttle, stop pedaling etc.) in percent per 10 millisecond.
// Specifying 1 will make ramp down periond 1 second if relasing from full throttle.
// Set to 100 to disable
#define CURRENT_RAMP_DOWN_PERCENT_10MS			5

// Target speed in km/h when walk mode is engaged
#define WALK_MODE_SPEED_KPH						4

// This value is used when assist level is configured with throttle cadence
// override flag in config tool. Default is 100%.
#define THROTTLE_CADENCE_OVERRIDE_PERCENT		100

// Lower limit for cadence rpm in power calculation
// for torque pas assist. When cadence is below this
// limit you will get extra power.
//
// power_w = torque_Nm * cadence_rpm * 0.105
//
// The calculated power is then multipled by a factor
// set by the assist level to get the final power which
// the motor will contribute.
//
// The value configured below is the minimum value for
// cadence_rpm to be used in the formula above. If the
// actual cadence is lower it will be overriden by this
// configured value.
#define TORQUE_POWER_LOWER_RPM_X10				300

#endif
