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

#define BATTERY_NO_LOAD_DELAY_MS		2000


static uint8_t compute_battery_percent()
{
	int16_t value_x100v = motor_get_battery_voltage_x10() * 10l;
	int16_t percent = (int16_t)MAP32(value_x100v, battery_empty_x100v, battery_full_x100v, 0, 100);

	return (uint8_t)CLAMP(percent, 0, 100);
}


void battery_init()
{
	battery_percent = 0;
	motor_disabled_at_ms = 0;

	// Consider battery full if above 98% of configured max voltage
	battery_full_x100v = (98l * EXPAND_U16(g_config.max_battery_x100v_u16h, g_config.max_battery_x100v_u16l)) / 100;

	// Consider battery empty at 5% above configured low voltage cutoff
	battery_empty_x100v = 105l * g_config.low_cut_off_v;
}

void battery_process()
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

uint8_t battery_get_percent()
{
	return battery_percent;
}
