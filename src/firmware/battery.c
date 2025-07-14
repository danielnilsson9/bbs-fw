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
#include "fwconfig.h"

static int16_t battery_empty_x100v;
static int16_t battery_full_x100v;

static uint8_t battery_percent;
static uint32_t motor_disabled_at_ms;
static bool first_reading_done;

/*
No attempt is made to have accurate battery state of charge display.

This is only a voltage based approch using configured max and min battery voltages.
The end values are padded 8% on each side (BATTERY_EMPTY_OFFSET_PERCENT, BATTERY_FULL_OFFSET_PERCENT).

Battery voltage is measured when no motor power has been applied for
at least 2 seconds (BATTERY_NO_LOAD_DELAY_MS). This is to mitigate measuring voltage sag
but is still problematic in cold weather.

Battery SOC percentage is calculated from measured voltage using linear interpolation
between the padded ranges.

The LVC rampdown starts at 10% battery SOC (LVC_RAMP_DOWN_OFFSET_PERCENT) and will linearly
ramp the current down to 20% (LVC_LOW_CURRENT_PERCENT) of the maximum configured current.

For example, if the maximum battery voltage is 58.8V and the low cutoff voltage is 42V, then:

- The full voltage range is 58.8V - 42V = 16.8V
- The padding amount is 0.08 * 16.8V = 1.3V
- The battery is considered at 100% SOC at 58.8V - 1.3V = 57.5V
- The battery is considered at   0% SOC at 42.0V + 1.3V = 43.3V
- LVC rampdown will start at 10% SOC, so: 43.3V + 0.1 * (57.5V - 43.3V) = 44.7V
- Full LVC limiting will occur at 0% SOC, so: 43.3V
*/

static uint8_t compute_battery_percent()
{
	int16_t value_x100v = motor_get_battery_voltage_x10() * 10l;
	int16_t percent = (int16_t)MAP32(value_x100v, battery_empty_x100v, battery_full_x100v, 0, 100);

	return (uint8_t)CLAMP(percent, 0, 100);
}

#if (BATTERY_PERCENT_MAP == BATTERY_PERCENT_MAP_SW102)
static uint8_t map_percent_sw102(uint8_t percent)
{
	// Measured on Display
	// -----------------------
	// 0bar		0-5
	// 1bar		5 - 10
	// 2bar		10 - 30
	// 3bar		31 - 51
	// 4bar		52 - 78
	// 5bar		78 - 100

	if (percent < 5)		// 0bar
	{
		return 0;
	}
	else if (percent < 21)	// 1bar
	{
		return 7;
	}
	else if (percent < 41)	// 2bar
	{
		return 20;
	}
	else if (percent < 61)	// 3bar
	{
		return 40;
	}
	else if (percent < 81)	// 4bar
	{
		return 60;
	}
	else					// 5bar
	{
		return 100;
	}
}
#endif


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
	uint16_t battery_full_pad_x100v = battery_range_x100v * BATTERY_FULL_OFFSET_PERCENT / 100;
	uint16_t battery_empty_pad_x100v = battery_range_x100v * BATTERY_EMPTY_OFFSET_PERCENT / 100;

	battery_full_x100v = battery_max_voltage_x100v - battery_full_pad_x100v;
	battery_empty_x100v = battery_min_voltage_x100v + battery_empty_pad_x100v;
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
			battery_percent = compute_battery_percent();
		}
	}
}

uint8_t battery_get_percent()
{
	return battery_percent;
}

uint8_t battery_get_mapped_percent()
{
#if (BATTERY_PERCENT_MAP == BATTERY_PERCENT_MAP_SW102)
	return map_percent_sw102(battery_percent);
#else
	return battery_percent;
#endif
}
