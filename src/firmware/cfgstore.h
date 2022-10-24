/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _CFGSTORE_H_
#define _CFGSTORE_H_

#include "intellisense.h"
#include <stdint.h>
#include <stdbool.h>

#define ASSIST_FLAG_PAS					0x01
#define ASSIST_FLAG_THROTTLE			0x02
#define ASSIST_FLAG_CRUISE				0x04
#define ASSIST_FLAG_PAS_VARIABLE		0x08	// pas mode using throttle to set power level
#define ASSIST_FLAG_PAS_TORQUE			0x10	// pas mode using torque sensor reading

#define ASSIST_MODE_SELECT_OFF			0x00
#define ASSIST_MODE_SELECT_STANDARD		0x01
#define ASSIST_MODE_SELECT_LIGHTS		0x02
#define ASSIST_MODE_SELECT_PAS0_LIGHT	0x03

#define TEMPERATURE_SENSOR_CONTR		0x01
#define TEMPERATURE_SENSOR_MOTOR		0x02

#define CONFIG_VERSION					3
#define PSTATE_VERSION					1

typedef struct
{
	uint8_t flags;
	uint8_t target_current_percent;
	uint8_t max_throttle_current_percent;
	uint8_t max_cadence_percent;
	uint8_t max_speed_percent;

	// 10 => 1.0: 100w human power gives and additional 100w motor power
	uint8_t torque_amplification_factor_x10;	
}  assist_level_t;

// SDCC uses little endian for MCS51 and big endian for STM8...
typedef struct
{
	// hmi units
	uint8_t use_freedom_units;

	// global
	uint8_t max_current_amps;
	uint8_t current_ramp_amps_s;
	uint8_t max_battery_x100v_u16l;
	uint8_t max_battery_x100v_u16h;
	uint8_t low_cut_off_v;
	uint8_t max_speed_kph;

	// externals
	uint8_t use_display;
	uint8_t use_speed_sensor;
	uint8_t use_shift_sensor;
	uint8_t use_push_walk;
	uint8_t use_temperature_sensor;

	// speed sensor
	uint8_t wheel_size_inch_x10_u16l;
	uint8_t wheel_size_inch_x10_u16h;
	uint8_t speed_sensor_signals;

	// pas options
	uint8_t pas_start_delay_pulses;
	uint8_t pas_stop_delay_x100s;
	uint8_t pas_keep_current_percent;
	uint8_t pas_keep_current_cadence_rpm;

	// throttle options
	uint8_t throttle_start_voltage_mv_u16l;
	uint8_t throttle_start_voltage_mv_u16h;
	uint8_t throttle_end_voltage_mv_u16l;
	uint8_t throttle_end_voltage_mv_u16h;
	uint8_t throttle_start_percent;

	// shift interrupt options
	uint16_t shift_interrupt_duration_ms;
	uint8_t shift_interrupt_current_threshold_percent;

	// misc
	uint8_t show_temperature_push_walk;

	// assist levels
	uint8_t assist_mode_select;
	uint8_t assist_startup_level;
	assist_level_t assist_levels[2][10];
} config_t;

typedef struct
{
	uint8_t adc_voltage_calibration_steps_x100_i16l;
	uint8_t adc_voltage_calibration_steps_x100_i16h;
} pstate_t;


extern config_t g_config;
extern pstate_t g_pstate;

void cfgstore_init();

bool cfgstore_reset_config();
bool cfgstore_save_config();

bool cfgstore_reset_pstate();
bool cfgstore_save_pstate();

#endif
