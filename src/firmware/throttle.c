/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "throttle.h"
#include "stc15.h"
#include "pins.h"
#include "system.h"
#include "eventlog.h"
#include "util.h"
#include "adc.h"

#include <stdbool.h>

static uint8_t min_voltage_adc;
static uint8_t max_voltage_adc;

static uint8_t start_percent;

static bool throttle_low_ok;
static bool throttle_hard_ok;
static uint32_t throttle_hard_limit_hit_at;


//#define LOG_THROTTLE_ADC

#define ADC_VOLTAGE_MV						5000ul

#define THROTTLE_HARD_LOW_LIMIT_MV			0500ul
#define THROTTLE_HARD_HIGH_LIMIT_MV			4500ul

#define THROTTLE_HARD_LOW_LIMIT_ADC			((THROTTLE_HARD_LOW_LIMIT_MV * 256) / ADC_VOLTAGE_MV)
#define THROTTLE_HARD_HIGH_LIMIT_ADC		((THROTTLE_HARD_HIGH_LIMIT_MV * 256) / ADC_VOLTAGE_MV)
#define THROTTLE_HARD_LIMIT_TOLERANCE_MS	100



void throttle_init(uint16_t min_mv, uint16_t max_mv)
{
	min_voltage_adc = (uint8_t)(((uint32_t)min_mv * 256) / ADC_VOLTAGE_MV);
	max_voltage_adc = (uint8_t)(((uint32_t)max_mv * 256) / ADC_VOLTAGE_MV);
	start_percent = 0;
	throttle_low_ok = false;
	throttle_hard_ok = true;
	throttle_hard_limit_hit_at = 0;
}

bool throttle_ok()
{
	return throttle_low_ok && throttle_hard_ok;
}

uint8_t throttle_read()
{
	int16_t value = adc_get_throttle();

#ifdef LOG_THROTTLE_ADC
	static uint8_t last_logged_throttle_adc = 0;	
	if (ABS(value - last_logged_throttle_adc) > 1)
	{
		last_logged_throttle_adc = value;
		eventlog_write_data(EVT_DATA_THROTTLE_ADC, value);		
	}
#endif
	
	if (value < THROTTLE_HARD_LOW_LIMIT_ADC || value > THROTTLE_HARD_HIGH_LIMIT_ADC)
	{
		// allow invalid throttle input value for a number of milliseconds before reporting throttle error.
		if (throttle_hard_limit_hit_at != 0)
		{
			if (throttle_hard_ok && (system_ms() - throttle_hard_limit_hit_at) > THROTTLE_HARD_LIMIT_TOLERANCE_MS)
			{
				if (value < THROTTLE_HARD_LOW_LIMIT_ADC)
				{
					eventlog_write(EVT_ERROR_THROTTLE_LOW_LIMIT);
				}
				else if (value > THROTTLE_HARD_HIGH_LIMIT_ADC)
				{
					eventlog_write(EVT_ERROR_THROTTLE_HIGH_LIMIT);
				}
				
				throttle_hard_ok = false;
			}
		}
		else
		{
			throttle_hard_limit_hit_at = system_ms();
		}
	}
	else
	{
		throttle_hard_limit_hit_at = 0;
		throttle_hard_ok = true;
	}

	if (value < min_voltage_adc)
	{
		// throttle is considered not working until it has given a signal below minimum value
		throttle_low_ok = true;
		return 0;
	}

	if (value > max_voltage_adc)
	{
		value = max_voltage_adc;
	}

	return (uint8_t)MAP16(value, min_voltage_adc, max_voltage_adc, 0, 100);
}

