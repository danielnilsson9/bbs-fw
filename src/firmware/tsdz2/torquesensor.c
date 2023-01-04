/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */
#include <stdint.h>
#include "system.h"
#include "sensors.h"
#include "adc.h"
#include "util.h"
#include "eventlog.h"
#include "tsdz2/stm8.h"
#include "tsdz2/pins.h"
#include "tsdz2/stm8s/stm8s_adc1.h"
#include "tsdz2/timers.h"


#define AUTO_BIAS_START_TIME_MS		2000
#define AUTO_BIAS_DURATION_MS		3000

// Hard coded default torque sensor calibration table for now
//
// Torque sensor readings on different TSDZ2 differs by a lot.
// This table is therefore not perfect for every motor but
// it will have to be good enough for now.
//
// Default firmware has no way to calibrate, no idea if calibrations
// is done at factory though.
//
// Consider adding manual calibration though config tool at some point.
// Until then, sensitivity can be set using torque amplification factor,
// which works well enough even if response will potentially not be linear.

#define TORQUE_SENSOR_LUT_SIZE 8

typedef struct { uint8_t adc; uint16_t nm_x100; } torque_lut_t;
static const torque_lut_t torque_sensor_lut[TORQUE_SENSOR_LUT_SIZE] =
{
	// (adc value - bias), (Nm x 100) 
	{ 0,   0     },	// 0kg
	{ 30,  834   },	// 5kg
	{ 55,  1668  },	// 10kg
	{ 78,  2502  },	// 15kg
	{ 93,  3169  },	// 19kg
	{ 188, 7004  },	// 42kg
	{ 204, 8672  },	// 52kg
	{ 224, 17511 }	// 105kg
};

static uint16_t torque_adc_to_nm_x100(uint16_t torque_adc)
{
	// interpolate in lookup table

	if (torque_adc < torque_sensor_lut[0].adc)
	{
		// use minimum value
		return torque_sensor_lut[0].nm_x100;
	}
	else if (torque_adc > torque_sensor_lut[TORQUE_SENSOR_LUT_SIZE - 1].adc)
	{
		// use maximum value
		return torque_sensor_lut[TORQUE_SENSOR_LUT_SIZE - 1].nm_x100;
	}

	uint8_t i = 0;
	for (i = 0; i < TORQUE_SENSOR_LUT_SIZE - 1; i++)
	{
		if (torque_sensor_lut[i + 1].adc > torque_adc)
		{
			break;
		}
	}

	return (uint16_t)MAP32(torque_adc,
		torque_sensor_lut[i].adc,
		torque_sensor_lut[i + 1].adc,
		torque_sensor_lut[i].nm_x100,
		torque_sensor_lut[i + 1].nm_x100);
}

static uint16_t torque_nm_x100 = 0;

static bool adc_bias_set = false;
static uint16_t adc_bias_steps = 0;


void torque_sensor_init()
{
	SET_PIN_OUTPUT_OPEN_DRAIN(PIN_TORQUE_SENSOR_EXC);

	timer2_init_torque_sensor_pwm();

	// some delay for torque sensor to power on
	system_delay_ms(50);
}

void torque_sensor_process()
{
	if (adc_bias_set)
	{
		uint16_t adc_val = adc_get_torque();
		if (adc_val > adc_bias_steps)
		{
			adc_val -= adc_bias_steps;
		}
		else
		{
			adc_val = 0;
		}

		// IDEA: Find max over pedal revolution period and use sin average (0.637)?
		// Doesn't seem to be needed, hw filtering seems to be very slow and should average just fine
		torque_nm_x100 = torque_adc_to_nm_x100(adc_val);
	}
	else
	{
		// find torque sensor adc bias during startup, torque sensor lookup table is relative to bias

		uint32_t now = system_ms();
		if (now < (AUTO_BIAS_START_TIME_MS + AUTO_BIAS_DURATION_MS))
		{
			if (now > AUTO_BIAS_START_TIME_MS)
			{
				uint16_t adc_val = adc_get_torque();
				if (adc_val > adc_bias_steps)
				{
					adc_bias_steps = adc_val;
				}
			}
		}
		else
		{
			adc_bias_set = true;
			eventlog_write_data(EVT_DATA_TORQUE_ADC_CALIBRATED, adc_bias_steps);
		}
	}
}

uint16_t torque_sensor_get_nm_x100()
{
	return torque_nm_x100;
}

bool torque_sensor_ok()
{
	return !adc_bias_set || adc_bias_steps > 50;
}
