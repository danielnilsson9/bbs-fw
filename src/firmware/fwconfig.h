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

#if defined(BBSHD) || defined(BBS02)
	#define HAS_SHIFT_SENSOR_SUPPORT			1
#else
	#define HAS_SHIFT_SENSOR_SUPPORT			0
#endif

#if defined(BBS02)
	#define MAX_CADENCE_RPM_X10					1500
#elif defined(BBSHD)
	// Measured on BBSHD at 48V
	#define MAX_CADENCE_RPM_X10					1680
#else
	#define MAX_CADENCE_RPM_X10					1200
#endif

#if defined(BBS02) || defined(BBSHD)
	#define PAS_PULSES_REVOLUTION				24
#elif defined(TSDZ2)
	#define PAS_PULSES_REVOLUTION				20
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

// No battery percent mapping
#define BATTERY_PERCENT_MAP_NONE				0
// Map battery percent to provide a linear relationship on the
// 5-bar battery indicator of the SW102 display.
#define BATTERY_PERCENT_MAP_SW102				1

// Select battery percent mapping
#define BATTERY_PERCENT_MAP						BATTERY_PERCENT_MAP_NONE

// Time with no motor load until battery voltage is updated to avoid voltage sag.
#define BATTERY_NO_LOAD_DELAY_MS		2000

// Padding values for voltage range of battery.
#define BATTERY_FULL_OFFSET_PERCENT		8
#define BATTERY_EMPTY_OFFSET_PERCENT	8

// Battery SOC percentage when current ramp down starts.
#define LVC_RAMP_DOWN_OFFSET_PERCENT			10

// Maximum allowed motor current in percent of maximum configured current (A)
// to still apply when 0% battery has been reached.
// Motor current is ramped down linearly until this value when approaching "empty".
#define LVC_LOW_CURRENT_PERCENT					20

// Size of speed limit ramp down interval.
// If max speed is 50 and this is set to 3 then the
// target current will start ramping down when passing 47
// and be at 50% of assist target current when reaching 50.
#define SPEED_LIMIT_RAMP_DOWN_INTERVAL_KPH		3

// Current ramp down (e.g. when releasing throttle, stop pedaling etc.) in percent per 10 millisecond.
// Specifying 1 will make ramp down periond 1 second if releasing from full throttle.
// Set to 100 to disable
#define CURRENT_RAMP_DOWN_PERCENT_10MS			5

// Target speed in km/h when walk mode is engaged
#define WALK_MODE_SPEED_KPH						4


#define THROTTLE_RESPONSE_LINEAR				1
#define THROTTLE_RESPONSE_QUADRATIC				2
#define THROTTLE_RESPONSE_CUSTOM				3

#define THROTTLE_RESPONSE_CURVE					THROTTLE_RESPONSE_CUSTOM

// Custom throttle map
// y = pow(x / 100.0, 1.5) * 100.0
#define THROTTLE_CUSTOM_MAP						\
	0,  1,  1,  1,  2,  2,  2,  3,  3,  3,		\
	4,  4,  4,  5,  5,  6,  6,  7,  8,  8,		\
	9,  10, 10, 11, 12, 12, 13, 14, 15, 16,		\
	16, 17, 18, 19, 20, 21, 22, 23, 23, 24,		\
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34,		\
	35, 36, 37, 39, 40, 41, 42, 43, 44, 45,		\
	46, 48, 49, 50, 51, 52, 54, 55, 56, 57,		\
	59, 60, 61, 62, 64, 65, 66, 68, 69, 70,		\
	72, 73, 74, 76, 77, 78, 80, 81, 83, 84,		\
	85, 87, 88, 90, 91, 93, 94, 96, 97, 99,		\
	100


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

// Number of PAS sensor pulses to engage cruise mode,
// there are 24 pulses per revolution.
#define CRUISE_ENGAGE_PAS_PULSES				PAS_PULSES_REVOLUTION / 2

// Number of PAS sensor pulses to disengage curise mode
// by pedaling backwards.
#define CRUISE_DISENGAGE_PAS_PULSES				PAS_PULSES_REVOLUTION / 2


// Option to control what data is displayed in "Range" field on display
// since range calculation is not implemented.
#define DISPLAY_RANGE_FIELD_ZERO				0
#define DISPLAY_RANGE_FIELD_TEMPERATURE			1	// max temperature of controller / motor
#define DISPLAY_RANGE_FIELD_POWER				2	// requested current x10 (lights off) / actual current x10 (lights on)

// uncomment and select option above
// #define DISPLAY_RANGE_FIELD_DATA		DISPLAY_RANGE_FIELD_ZERO

// default to temperature if temperature sensors available (BBS2/BBSHD), else power (TSDZ2)
#ifndef DISPLAY_RANGE_FIELD_DATA
	#if HAS_CONTROLLER_TEMP_SENSOR || HAS_MOTOR_TEMP_SENSOR
	#define DISPLAY_RANGE_FIELD_DATA		DISPLAY_RANGE_FIELD_TEMPERATURE
	#else
	#define DISPLAY_RANGE_FIELD_DATA		DISPLAY_RANGE_FIELD_POWER
	#endif
#endif

// Eggrider compatibility. When the display sets the speed limit
// to a certain value, use sport mode. The actual speed limit of the motor
// remains unchanged. This is a workaround; it is not possible to
// program the controller using an Eggrider as it doesn't understand the
// bbs-fw protocol. App configuration:
// - Set "Bafang switch mode" to "Only live data"
// - Make sure the bbs-fw wheel diameter in inches matches the Eggrider
//   wheel circumference in mm. Formula:
//   circumference (mm) = diameter * π * 25.4
// - Set "Max speed OffRoad" to the value set below.
#define SPEED_LIMIT_SPORT_SWITCH_KPH 			0 // speed in km/h. Setting to 0 disables this feature

#endif
