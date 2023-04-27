/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "battery.h"
#include "motor.h"
#include "system.h"
#include "util.h"
#include "cfgstore.h"

static int16_t battery_empty_x100v;
static int16_t battery_full_x100v;

static uint8_t battery_percent;
static uint32_t motor_disabled_at_ms;
static bool first_reading_done;

#define BATTERY_NO_LOAD_DELAY_MS		2000

#define BATTERY_FULL_OFFSET_PERCENT		8
#define BATTERY_EMPTY_OFFSET_PERCENT	8

//
// No attempt is made to have accurate battery state of charge display.
//
// This is only a voltage based approch using configred max and min battery voltages.
// The end values are padded (BATTERY_EMPTY_OFFSET_PERCENT, BATTERY_FULL_OFFSET_PERCENT).
//
// Battery voltage is measured when no motor power has been applied for at least 2 seconds
// (to mitigate measuring voltage sag but is still problematic in cold weather).
//
// Battery SOC percentage is calculated from measured voltage using linear interpolation.
//


static uint8_t compute_battery_percent()
{
	int16_t value_x100v = motor_get_battery_voltage_x10() * 10l;
	int16_t percent = (int16_t)MAP32(value_x100v, battery_empty_x100v, battery_full_x100v, 0, 100);

	return (uint8_t)CLAMP(percent, 0, 100);
}


void battery_init()
{
	// default to 70% until first reading is available
	battery_percent = 70;
	motor_disabled_at_ms = 0;
	first_reading_done = false;

	uint16_t battery_min_voltage_x100v = g_config.low_cut_off_v * 100u;
	uint16_t battery_max_voltage_x100v =
		EXPAND_U16(g_config.max_battery_x100v_u16h, g_config.max_battery_x100v_u16l);

	uint16_t battery_range_x100v = battery_max_voltage_x100v - battery_min_voltage_x100v;

	// Consider battery full if above 95% (100 - BATTERY_FULL_OFFSET_PERCENT)
	// of configured voltage range
	battery_full_x100v = battery_max_voltage_x100v -
		((BATTERY_FULL_OFFSET_PERCENT * battery_range_x100v) / 100);

	// Consider battery empty if below 8% (BATTERY_EMPTY_OFFSET_PERCENT)
	// of configured voltage range
	battery_empty_x100v = battery_min_voltage_x100v +
		((BATTERY_EMPTY_OFFSET_PERCENT * battery_range_x100v) / 100);
}

void battery_process()
{
	if (!first_reading_done)
	{
		if (motor_get_battery_voltage_x10() > 0)
		{
			battery_percent = compute_battery_percent();
			first_reading_done = true;
		}
	}
	else
	{
		uint8_t target_current = motor_get_target_current();

		if (motor_disabled_at_ms == 0 && target_current == 0)
		{
			motor_disabled_at_ms = system_ms();
		}
		else if (target_current > 0)
		{
			motor_disabled_at_ms = 0;
		}

		if (target_current == 0 && (system_ms() - motor_disabled_at_ms) > BATTERY_NO_LOAD_DELAY_MS)
		{
			// Compute battery percent using linear interpolation between lvc
			// and configured max voltage while under no load.

			battery_percent = compute_battery_percent();
		}
	}
}

uint8_t battery_get_percent()
{
	return battery_percent;
}
