/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#ifndef _CFGSTORE_H_
#define _CFGSTORE_H_

#include <stdint.h>
#include <stdbool.h>

#define ASSIST_FLAG_PAS					0x01
#define ASSIST_FLAG_THROTTLE			0x02
#define ASSIST_FLAG_CRUISE				0x04

#define ASSIST_MODE_SELECT_OFF			0x00
#define ASSIST_MODE_SELECT_STANDARD		0x01
#define ASSIST_MODE_SELECT_LIGHTS		0x02
#define ASSIST_MODE_SELECT_PAS0_LIGHT	0x03


#define CONFIG_VERSION					1


typedef struct
{
	uint8_t flags;
	uint8_t target_current_percent;
	uint8_t max_throttle_current_percent;
	uint16_t max_speed_percent;
} assist_level_t;

typedef struct
{
	// hmi units
	uint8_t use_freedom_units;

	// power
	uint8_t max_current_amps;
	uint8_t low_cut_off_V;

	// externals
	uint8_t use_speed_sensor;
	uint8_t use_display;
	uint8_t use_push_walk;

	// speed sensor
	uint16_t wheel_size_inch_x10;
	uint8_t speed_sensor_signals;
	uint8_t max_speed_kph;

	// pas options
	uint8_t pas_start_delay_pulses;
	uint8_t pas_stop_delay_ms_x10;

	// throttle options
	uint16_t throttle_start_voltage_mv;
	uint16_t throttle_end_voltage_mv;
	uint8_t throttle_start_percent;

	// assist options
	uint8_t assist_mode_select;
	uint8_t assist_startup_level;
	assist_level_t assist_levels[2][10];
} config_t;



void cfgstore_init();

config_t* cfgstore_get();
bool cfgstore_save();

#endif
