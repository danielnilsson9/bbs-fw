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
#include "system.h"

static __xdata uint8_t assist_level;
static __xdata uint8_t operation_mode;
static __xdata uint16_t global_max_speed_rpm;

static __xdata assist_level_t assist_level_data;
static __xdata int32_t assist_max_wheel_speed_rpm_x10;
static __xdata uint16_t speed_limit_ramp_interval_rpm_x10;

static __xdata bool last_light_state;
static __xdata bool cruise_paused;
static __xdata bool cruise_block_throttle_return;

static __xdata int8_t last_temperature;
static __xdata bool speed_limiting;

static __xdata uint8_t ramp_up_target_current;
static __xdata uint32_t last_ramp_up_increment_ms;
static __xdata uint16_t ramp_up_current_interval_ms;

#define MAX_TEMPERATURE						70
#define CRUISE_ENGAGE_PAS_PULSES			12
#define SPEED_LIMIT_RAMP_DOWN_INTERVAL_KPH	2

#define CURRENT_RAMP_UP_AMPS_SECOND_X10		150


void apply_pas(uint8_t* target_current);
void apply_cruise(uint8_t* target_current, uint8_t throtle_percent);
void apply_throttle(uint8_t* target_current, uint8_t throttle_percen);
void apply_current_ramp(uint8_t* target_current);
void apply_speed_limit(uint8_t* target_current);
void apply_thermal_limit(uint8_t* target_current);

void reload_assist_params();

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph);

void app_init()
{
	motor_disable();
	lights_disable();

	global_max_speed_rpm = 0;
	last_light_state = false;
	last_temperature = 0;
	speed_limiting = false;

	ramp_up_target_current = 0;
	ramp_up_current_interval_ms = (g_config.max_current_amps * 100u) / CURRENT_RAMP_UP_AMPS_SECOND_X10;
	last_ramp_up_increment_ms = 0;

	cruise_paused = true;
	cruise_block_throttle_return = false;
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
		}
	}
	else
	{
		uint8_t throttle = throttle_read();

		apply_pas(&target_current);
		apply_cruise(&target_current, throttle);
		apply_current_ramp(&target_current);	// order important, shall not affect throttle

		apply_throttle(&target_current, throttle);
	}

	apply_speed_limit(&target_current);
	apply_thermal_limit(&target_current);

	motor_set_target_speed(255u * assist_level_data.max_cadence_percent / 100u);
	motor_set_target_current(target_current);
	
	if (target_current > 0 && !brake_is_activated() && !gear_sensor_is_activated())
	{
		motor_enable();
	}
	else
	{
		motor_disable();

		// force reset current ramp
		ramp_up_target_current = 0;
		last_ramp_up_increment_ms = 0;
	}

	if (motor_status() & MOTOR_ERROR_LVC)
	{
		lights_disable();
	}
	else
	{
		lights_enable();
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
	if (
		(g_config.assist_mode_select == ASSIST_MODE_SELECT_LIGHTS) ||
		(assist_level == ASSIST_0 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS0_LIGHT)
	)
	{
		if (on)
		{
			app_set_operation_mode(OPERATION_MODE_SPORT);
		}
		else
		{
			app_set_operation_mode(OPERATION_MODE_DEFAULT);
		}
	}
	else
	{
		if (last_light_state != on)
		{
			last_light_state = on;
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

uint8_t app_get_status_code()
{
	uint16_t motor = motor_status();

	// TODO: figure out current sense error code from motor controller

	if (motor & 0x10)
	{
		return STATUS_ERROR_HALL_SENSOR;
	}

	if (!throttle_ok())
	{
		return STATUS_ERROR_THROTTLE;
	}

	if (last_temperature > MAX_TEMPERATURE)
	{
		return STATUS_ERROR_CONTROLLER_OVER_TEMP;
	}

	if (motor & MOTOR_ERROR_LVC)
	{
		return STATUS_ERROR_LVC;
	}

	if (brake_is_activated() || gear_sensor_is_activated())
	{
		return STATUS_BRAKING;
	}

	if (pas_is_pedaling_forwards())
	{
		return STATUS_PEDALING;
	}

	return STATUS_IDLE;
}

uint8_t app_get_motor_temperature()
{
	return last_temperature;
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
	if ((assist_level_data.flags & ASSIST_FLAG_CRUISE) && throttle_ok())
	{
		// pause cruise if brake activated
		if (brake_is_activated())
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// pause cruise if started pedaling backwards
		else if (pas_is_pedaling_backwards() && pas_get_pulse_counter() > CRUISE_ENGAGE_PAS_PULSES)
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// pause cruise if throttle touched while cruise active
		else if (!cruise_paused && !cruise_block_throttle_return && throttle_percent > 0)
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// unpause cruise if pedaling forward while engaging throttle > 50%
		else if (cruise_paused && !cruise_block_throttle_return && throttle_percent > 50 && pas_is_pedaling_forwards() && pas_get_pulse_counter() > CRUISE_ENGAGE_PAS_PULSES)
		{
			cruise_paused = false;
			cruise_block_throttle_return = true;
		}

		// reset flag tracking throttle to make sure throttle returns to idle position before engage/disenage cruise with throttle touch
		else if (cruise_block_throttle_return && throttle_percent == 0)
		{
			cruise_block_throttle_return = false;
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

void apply_current_ramp(uint8_t* target_current)
{
	if (*target_current > ramp_up_target_current)
	{
		__xdata uint32_t now = system_ms();
		__xdata uint16_t time_diff = now - last_ramp_up_increment_ms;

		if (time_diff >= ramp_up_current_interval_ms)
		{
			++ramp_up_target_current;
		
			if (last_ramp_up_increment_ms == 0)
			{
				last_ramp_up_increment_ms = now;
			}
			else
			{
				// offset for time overshoot to not accumulate large ramp error
				last_ramp_up_increment_ms = now - (uint8_t)(time_diff - ramp_up_current_interval_ms);
			}		
		}

		*target_current = ramp_up_target_current;
	}
	else
	{
		ramp_up_target_current = *target_current;
	}
}

void apply_throttle(uint8_t* target_current, uint8_t throttle_percent)
{
	if ((assist_level_data.flags & ASSIST_FLAG_THROTTLE) && throttle_percent > 0 && throttle_ok())
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
	if (g_config.use_speed_sensor && assist_max_wheel_speed_rpm_x10 > 0)
	{
		int16_t current_speed_rpm_x10 = speed_sensor_get_rpm_x10();

		int16_t high_limit_rpm = assist_max_wheel_speed_rpm_x10 + speed_limit_ramp_interval_rpm_x10;
		int16_t low_limit_rpm = assist_max_wheel_speed_rpm_x10 - speed_limit_ramp_interval_rpm_x10;
	
		if (current_speed_rpm_x10 < low_limit_rpm)
		{
			// no limiting
			if (speed_limiting)
			{
				speed_limiting = false;
				eventlog_write_data(EVT_DATA_SPEED_LIMITING, 0);
			}
		}		
		else
		{
			if (current_speed_rpm_x10 > high_limit_rpm)
			{
				if (*target_current > 1)
				{
					*target_current = 1;
				}
			}
			else
			{
				// linear ramp down when approaching max speed.
				uint8_t tmp = (uint8_t)MAP(current_speed_rpm_x10, low_limit_rpm, high_limit_rpm, *target_current, 1);
				if (*target_current > tmp)
				{
					*target_current = tmp;
				}
			}

			if (!speed_limiting)
			{
				speed_limiting = true;
				eventlog_write_data(EVT_DATA_SPEED_LIMITING, 1);
			}
		}
	}
}

void apply_thermal_limit(uint8_t* target_current)
{
	int8_t temp = temperature_read();

	if (temp != last_temperature)
	{
		last_temperature = temp;
		eventlog_write_data(EVT_DATA_TEMPERATURE, temp);
	}

	if (temp > MAX_TEMPERATURE)
	{
		if (last_temperature < MAX_TEMPERATURE)
		{
			eventlog_write_data(EVT_DATA_THERMAL_LIMITING, 1);
		}

		*target_current = *target_current / 2;
	}
	else
	{
		if (last_temperature > MAX_TEMPERATURE)
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
		assist_max_wheel_speed_rpm_x10 = ((uint32_t)((uint32_t)global_max_speed_rpm * assist_level_data.max_speed_percent) / 10);

		// pause cruise if swiching level
		cruise_paused = true;
	}
	else if (assist_level == ASSIST_PUSH)
	{
		assist_level_data.flags = 0;
		assist_level_data.target_current_percent = 0;
		assist_level_data.max_speed_percent = 0;
		assist_level_data.max_cadence_percent = 15;
		assist_level_data.max_throttle_current_percent = 0;
		
		assist_max_wheel_speed_rpm_x10 = convert_wheel_speed_kph_to_rpm(6) * 10;
	}

	speed_limit_ramp_interval_rpm_x10 = convert_wheel_speed_kph_to_rpm(SPEED_LIMIT_RAMP_DOWN_INTERVAL_KPH) * 10;
}

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph)
{
	float radius_mm = g_config.wheel_size_inch_x10 * 1.27f; // g_config.wheel_size_inch_x10 / 2.f * 2.54f;
	return (uint16_t)(25000.f / (3 * 3.14159f * radius_mm) * speed_kph);
}

