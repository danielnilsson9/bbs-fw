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

static uint8_t battery_percent;
static uint32_t motor_disabled_at_ms;

#define BATTERY_NO_LOAD_DELAY_MS		2000


static uint8_t compute_battery_percent()
{
	// Compute battery percent using stupid linear approximation between lvc and configure max voltage under no load (not correct).

	// Consider battery full if above 98% of configure max voltage
	int32_t full_x1000v = 98l * g_config.max_battery_x100v * 10 / 100;

	int32_t val = MAP32(motor_get_battery_voltage_x10() * 100l, g_config.low_cut_off_v * 1000l, full_x1000v, 0, 100);

	if (val > 100)
	{
		val = 100;
	}
	else if (val < 0)
	{
		val = 0;
	}

	return (uint8_t)val;
}


void battery_init()
{
	battery_percent = 0;
	motor_disabled_at_ms = 0;
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
		battery_percent = compute_battery_percent();
	}
}

uint8_t battery_get_percent()
{
	return battery_percent;
}
