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
#include "uart.h"
#include "eventlog.h"

static uint8_t __xdata assist_level;
static uint8_t __xdata operation_mode;
static uint16_t __xdata global_max_speed_ppm;

static assist_level_t __xdata assist_level_data;

void reload_assist_params();


void app_init()
{
	motor_disable();

	config_t* cfg = cfgstore_get();

	operation_mode = OPERATION_MODE_DEFAULT;
	global_max_speed_ppm = 65535;
	app_set_assist_level(cfg->assist_startup_level);
}

void app_process()
{
	config_t* cfg = cfgstore_get();

	uint8_t tmp;
	uint8_t target_current = 0;

	if (assist_level == ASSIST_PUSH)
	{
		target_current = 10;
		motor_set_target_speed(40);
	}
	else
	{
		motor_set_target_speed(0xff);

		if (assist_level_data.flags & ASSIST_FLAG_CRUISE)
		{
			target_current = assist_level_data.target_current_percent;
			// :TODO: implement must pedal one loop to activate and pedal backwards to deatctivate
		}
		else if (assist_level_data.flags & ASSIST_FLAG_PAS)
		{
			if (pas_is_pedaling_forwards() && pas_get_pulse_counter() > cfg->pas_start_delay_pulses)
			{
				target_current = assist_level_data.max_throttle_current_percent;
			}
		}

		if (assist_level_data.flags & ASSIST_FLAG_THROTTLE)
		{
			tmp = (uint8_t)(((uint16_t)throttle_read() * assist_level_data.max_throttle_current_percent) / 100);
			if (tmp > target_current)
			{
				target_current = tmp;
			}
		}
	}

	motor_set_target_current(target_current);
	
	if (target_current > 0)
	{
		motor_enable();
	}
	else
	{
		motor_disable();
	}
	
	if (brake_is_activated() || pas_is_pedaling_backwards())
	{
		motor_disable();
	}
}



void app_set_assist_level(uint8_t level)
{
	assist_level = level;
	eventlog_write_data(EVT_DATA_ASSIST_LEVEL, assist_level);
	reload_assist_params();
}

void app_set_lights(bool on)
{
	config_t* cfg = cfgstore_get();

	if ((cfg->assist_mode_select & ASSIST_MODE_SELECT_LIGHTS) ||
		(assist_level == ASSIST_0 && cfg->assist_mode_select & ASSIST_MODE_SELECT_PAS0_LIGHT))
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
		// :TODO: set lights on/off
	}
}

void app_set_operation_mode(uint8_t mode)
{
	operation_mode = mode;
	eventlog_write_data(EVT_DATA_OPERATION_MODE, operation_mode);
	reload_assist_params();
}

void app_set_wheel_max_speed_ppm(uint16_t value)
{
	global_max_speed_ppm = value;
	eventlog_write_data(EVT_DATA_WHEEL_SPEED_PPM, value);
	reload_assist_params();
}


void reload_assist_params()
{
	if (assist_level < ASSIST_PUSH)
	{
		assist_level_data = cfgstore_get()->assist_levels[operation_mode][assist_level];
		// :TODO: update pid controller with target max speed
	}
	else if (assist_level == ASSIST_PUSH)
	{
		assist_level_data.flags = 0;
		assist_level_data.target_current_percent = 0;
		assist_level_data.max_speed_percent = 0;
		assist_level_data.max_throttle_current_percent = 0;

		// :TODO: update pid controller with target max speed
	}
}


