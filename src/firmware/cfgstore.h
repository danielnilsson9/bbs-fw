/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _CFGSTORE_H_
#define _CFGSTORE_H_

#include "stc15.h"
#include <stdint.h>
#include <stdbool.h>

#define ASSIST_FLAG_PAS					0x01
#define ASSIST_FLAG_THROTTLE			0x02
#define ASSIST_FLAG_CRUISE				0x04
#define ASSIST_FLAG_VARPAS				0x08

#define ASSIST_MODE_SELECT_OFF			0x00
#define ASSIST_MODE_SELECT_STANDARD		0x01
#define ASSIST_MODE_SELECT_LIGHTS		0x02
#define ASSIST_MODE_SELECT_PAS0_LIGHT	0x03

#define TEMPERATURE_SENSOR_CONTR		0x01
#define TEMPERATURE_SENSOR_MOTOR		0x02

#define CONFIG_VERSION					2

typedef struct
{
	uint8_t flags;
	uint8_t target_current_percent;
	uint8_t max_throttle_current_percent;
	uint8_t max_cadence_percent;
	uint8_t max_speed_percent;
}  assist_level_t;

typedef struct
{
	// hmi units
	uint8_t use_freedom_units;

	// global
	uint8_t max_current_amps;
	uint8_t current_ramp_amps_s;
	uint16_t max_battery_x100v;
	uint8_t low_cut_off_v;
	uint8_t max_speed_kph;

	// externals
	uint8_t use_speed_sensor;
	uint8_t use_display;
	uint8_t use_push_walk;
	uint8_t use_temperature_sensor;

	// speed sensor
	uint16_t wheel_size_inch_x10;
	uint8_t speed_sensor_signals;

	// pas options
	uint8_t pas_start_delay_pulses;
	uint8_t pas_stop_delay_x100s;

	// throttle options
	uint16_t throttle_start_voltage_mv;
	uint16_t throttle_end_voltage_mv;
	uint8_t throttle_start_percent;

	// misc
	uint8_t show_temperature_push_walk;

	// assist levels
	uint8_t assist_mode_select;
	uint8_t assist_startup_level;
	assist_level_t assist_levels[2][10];
} config_t;


extern config_t g_config;

void cfgstore_init();

bool cfgstore_reset();
bool cfgstore_save();

#endif
