/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "app.h"
#include "stc15.h"
#include "cfgstore.h"
#include "motor.h"
#include "sensors.h"
#include "throttle.h"
#include "lights.h"
#include "uart.h"
#include "eventlog.h"
#include "util.h"

static uint8_t __xdata assist_level;
static uint8_t __xdata operation_mode;
static uint16_t __xdata global_max_speed_rpm;

static assist_level_t __xdata assist_level_data;
static int32_t __xdata assist_max_wheel_speed_rpm_x10;

static bool __xdata last_light_state;
static bool __xdata cruise_paused;
static bool __xdata cruise_just_engaged;

static uint8_t __xdata last_temperature;


void apply_pas(uint8_t* target_current);
void apply_cruise(uint8_t* target_current, uint8_t throtle_percent);
void apply_throttle(uint8_t* target_current, uint8_t throttle_percen);
void apply_speed_limit(uint8_t* target_current);
void apply_thermal_limit(uint8_t* target_current);

void reload_assist_params();

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph);

void app_init()
{
	motor_disable();

	last_light_state = false;
	last_temperature = 0;

	cruise_paused = true;
	cruise_just_engaged = false;
	operation_mode = OPERATION_MODE_DEFAULT;
	app_set_wheel_max_speed_rpm(convert_wheel_speed_kph_to_rpm(g_config.max_speed_kph));
	app_set_assist_level(g_config.assist_startup_level);
	reload_assist_params();
}

void app_process()
{
	uint8_t target_current = 0;

	if (assist_level == ASSIST_PUSH)
	{
		if (g_config.use_push_walk)
		{
			target_current = 10;
			motor_set_target_speed(40);
		}		
	}
	else
	{
		// never limit motor rotation speed
		motor_set_target_speed(0xff);

		uint8_t throttle = throttle_read();

		apply_pas(&target_current);
		apply_cruise(&target_current, throttle);
		apply_throttle(&target_current, throttle);	
	}

	apply_speed_limit(&target_current);
	apply_thermal_limit(&target_current);

	motor_set_target_current(target_current);
	
	if (target_current > 0 && !brake_is_activated())
	{
		motor_enable();
	}
	else
	{
		motor_disable();
	}
}


void app_set_assist_level(uint8_t level)
{
	if (assist_level != level)
	{
		assist_level = level;
		eventlog_write_data(EVT_DATA_ASSIST_LEVEL, assist_level);
		reload_assist_params();
	}
}

void app_set_lights(bool on)
{
	if (last_light_state != on)
	{
		last_light_state = on;

		if ((g_config.assist_mode_select & ASSIST_MODE_SELECT_LIGHTS) ||
			(assist_level == ASSIST_0 && g_config.assist_mode_select & ASSIST_MODE_SELECT_PAS0_LIGHT))
		{
			if (on)
			{
				operation_mode = OPERATION_MODE_SPORT;
			}
			else
			{
				operation_mode = OPERATION_MODE_DEFAULT;
			}

			eventlog_write_data(EVT_DATA_OPERATION_MODE, operation_mode);

			reload_assist_params();
		}
		else
		{
			eventlog_write_data(EVT_DATA_LIGHTS, on);
			lights_set(on);
		}
	}
}

void app_set_operation_mode(uint8_t mode)
{
	if (operation_mode != mode)
	{
		operation_mode = mode;
		eventlog_write_data(EVT_DATA_OPERATION_MODE, operation_mode);
		reload_assist_params();
	}
}

void app_set_wheel_max_speed_rpm(uint16_t value)
{
	if (global_max_speed_rpm != value)
	{
		global_max_speed_rpm = value;
		eventlog_write_data(EVT_DATA_WHEEL_SPEED_PPM, value);
		reload_assist_params();
	}
}


void apply_pas(uint8_t* target_current)
{
	if (assist_level_data.flags & ASSIST_FLAG_PAS)
	{
		if (pas_is_pedaling_forwards() && pas_get_pulse_counter() > g_config.pas_start_delay_pulses)
		{
			if (assist_level_data.target_current_percent > *target_current)
			{
				*target_current = assist_level_data.target_current_percent;
			}		
		}
	}
}

void apply_cruise(uint8_t* target_current, uint8_t throttle_percent)
{
	if (assist_level_data.flags & ASSIST_FLAG_CRUISE)
	{
		// pause cruise if started pedaling backwards
		if (pas_is_pedaling_backwards() && pas_get_pulse_counter() > g_config.pas_start_delay_pulses)
		{
			cruise_paused = true;
			cruise_just_engaged = false;
		}

		// pause cruise if throttle touched while cruise active
		else if (!cruise_paused && !cruise_just_engaged && throttle_percent > 0)
		{
			cruise_paused = true;
			cruise_just_engaged = false;
		}

		// unpause cruise if pedaling forward while engaging throttle > 50%
		else if (cruise_paused && throttle_percent > 50 && pas_is_pedaling_forwards() && pas_get_pulse_counter() > g_config.pas_start_delay_pulses)
		{
			cruise_paused = false;
			cruise_just_engaged = true;
		}

		// reset flag tracking throttle to make sure throttle returns to idle position before disabling cruise with throttle touch
		else if (cruise_just_engaged && throttle_percent == 0)
		{
			cruise_just_engaged = false;
		}

		if (cruise_paused)
		{
			*target_current = 0;
		}
		else
		{
			if (assist_level_data.target_current_percent > *target_current)
			{
				*target_current = assist_level_data.target_current_percent;
			}
		}
	}
}

void apply_throttle(uint8_t* target_current, uint8_t throttle_percent)
{
	if (assist_level_data.flags & ASSIST_FLAG_THROTTLE && throttle_percent > 0)
	{
		uint8_t current = (uint8_t)MAP(throttle_percent, 0, 100, g_config.throttle_start_percent, assist_level_data.max_throttle_current_percent);
		if (current > *target_current)
		{
			*target_current = current;
		}
	}
}

void apply_speed_limit(uint8_t* target_current)
{
	if (g_config.use_speed_sensor)
	{
		int16_t current_speed_x10 = speed_sensor_get_rpm_x10();

		int16_t high_limit = assist_max_wheel_speed_rpm_x10 + 20;
		int16_t low_limit = assist_max_wheel_speed_rpm_x10 - 20;

		if (current_speed_x10 > high_limit)
		{
			*target_current = 0;
		}
		else if (current_speed_x10 < low_limit)
		{
			// no limiting
		}
		else
		{
			// linear ramp down when approaching max speed.
			*target_current = (uint8_t)MAP(current_speed_x10, low_limit, high_limit, *target_current, 2);
		}
	}
}

void apply_thermal_limit(uint8_t* target_current)
{
	uint8_t temp = temperature_read();

	if (temp != last_temperature)
	{
		last_temperature = temp;
		eventlog_write_data(EVT_DATA_TEMPERATURE, temp);
	}

	if (temp > 80)
	{
		if (last_temperature < 80)
		{
			eventlog_write_data(EVT_DATA_THERMAL_LIMITING, 1);
		}

		*target_current = *target_current / 2;
	}
	else
	{
		if (last_temperature > 80)
		{
			eventlog_write_data(EVT_DATA_THERMAL_LIMITING, 0);
		}
	}
}


void reload_assist_params()
{
	if (assist_level < ASSIST_PUSH)
	{
		assist_level_data = g_config.assist_levels[operation_mode][assist_level];
		assist_max_wheel_speed_rpm_x10 = ((uint32_t)global_max_speed_rpm * assist_level_data.max_speed_percent / 10);

		// pause cruise if swiching level
		cruise_paused = true;
	}
	else if (assist_level == ASSIST_PUSH)
	{
		assist_level_data.flags = 0;
		assist_level_data.target_current_percent = 0;
		assist_level_data.max_speed_percent = 0;
		assist_level_data.max_throttle_current_percent = 0;

		assist_max_wheel_speed_rpm_x10 = convert_wheel_speed_kph_to_rpm(6) * 10;
	}
}

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph)
{
	float radius_mm = g_config.wheel_size_inch_x10 * 2.54f;
	return (uint16_t)(25000.f / (3 * 3.14159f * radius_mm) * speed_kph);
}

