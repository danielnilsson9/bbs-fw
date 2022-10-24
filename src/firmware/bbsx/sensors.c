/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "sensors.h"
#include "system.h"
#include "adc.h"
#include "util.h"
#include "cfgstore.h"
#include "eventlog.h"
#include "bbsx/pins.h"
#include "bbsx/stc15.h"
#include "bbsx/timers.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// interrupt runs at 100us interval, see timer0 in timers.c
// timer0 is shared between system and sensors modules

#define PAS_SENSOR_NUM_SIGNALS			24
#define PAS_SENSOR_MIN_PULSE_MS_X10		50	// 500rpm limit

#define SPEED_SENSOR_MIN_PULSE_MS_X10	500
#define SPEED_SENSOR_TIMEOUT_MS_X10		25000


// Some versions of the BBSHD motor (hall sensor board)
// has a PTC thermistor instead of a NTC thermistor.
// Using standard PT1000 table.
// [R_x100, C_x100]

#ifdef BBSHD
#define BBSHD_PTC_LUT_SIZE	21
typedef struct { int32_t x; int16_t y; } pt_t;
static const pt_t bbshd_ptc_lut[BBSHD_PTC_LUT_SIZE] =
{
	{ 92100, -2000 },
	{ 96090, -1000 },
	{ 100000, 0000 },
	{ 103900, 1000 },
	{ 107790, 2000 },
	{ 109730, 2500 },
	{ 111670, 3000 },
	{ 113610, 3500 },
	{ 115540, 4000 },
	{ 117470, 4500 },
	{ 119400, 5000 },
	{ 121320, 5500 },
	{ 123240, 6000 },
	{ 125160, 6500 },
	{ 127080, 7000 },
	{ 128990, 7500 },
	{ 130900, 8000 },
	{ 132800, 8500 },
	{ 134710, 9000 },
	{ 136610, 9500 },
	{ 138510, 10000 }
};
static bool bbshd_ptc_thermistor;
#endif


static volatile uint16_t pas_pulse_counter;
static volatile bool pas_direction_backward;
static volatile uint16_t pas_period_length;	// pulse length counted in interrupt frequency (100us)
static uint16_t pas_period_counter;
static bool pas_prev1;
static bool pas_prev2;
static uint16_t pas_stop_delay_periods;

static volatile uint16_t speed_ticks_period_length; // pulse length counted in interrupt frequency (100us)
static uint16_t speed_period_counter;
static bool speed_prev_state;
static uint8_t speed_ticks_per_rpm;


static float thermistor_ntc_calculate_temperature(float R, float invBeta)
{
	const float invT0 = 1.f / 298.15f;

	float K = 1.f / (invT0 + invBeta * (logf(R / 10000.f)));
	float C = K - 273.15f;

	return C;
}

#ifdef BBSHD
static int16_t thermistor_ptc_bbshd_calculate_temperature(int32_t R_x100)
{
	// interpolate in lookup table

	if (R_x100 < bbshd_ptc_lut[0].x)
	{
		// use minimum value
		return bbshd_ptc_lut[0].y;
	}
	else if (R_x100 > bbshd_ptc_lut[BBSHD_PTC_LUT_SIZE - 1].x)
	{
		// use maximum value
		return bbshd_ptc_lut[BBSHD_PTC_LUT_SIZE - 1].y;
	}

	uint8_t i = 0;
	for (i = 0; i < BBSHD_PTC_LUT_SIZE - 1; i++)
	{
		if (bbshd_ptc_lut[i + 1].x > R_x100)
		{
			break;
		}
	}

	return (uint16_t)MAP32(R_x100,
		bbshd_ptc_lut[i].x,
		bbshd_ptc_lut[i + 1].x,
		bbshd_ptc_lut[i].y,
		bbshd_ptc_lut[i + 1].y);
}
#endif


void sensors_init()
{
	// will be evaulated when first reading take place
#ifdef BBSHD
	bbshd_ptc_thermistor = false;
#endif

	pas_period_counter = 0;
	pas_pulse_counter = 0;
	pas_direction_backward = false;
	pas_period_length = 0;
	pas_stop_delay_periods = 1500;
	speed_period_counter = 0;
	speed_ticks_period_length = 0;
	speed_prev_state = false;
	speed_ticks_per_rpm = 1;

	// pins do not have external interrupt, use timer0 to evaluate state frequently
	SET_PIN_INPUT(PIN_PAS1);
	SET_PIN_INPUT(PIN_PAS2);
	SET_PIN_INPUT(PIN_SPEED_SENSOR);

	SET_PIN_QUASI(PIN_BRAKE); // input pullup
	SET_PIN_QUASI(PIN_SHIFT_SENSOR); // input pullup

	pas_prev1 = GET_PIN_STATE(PIN_PAS1);
	pas_prev2 = GET_PIN_STATE(PIN_PAS2);

	timer0_init_sensors();
}

void sensors_process()
{

}

void pas_set_stop_delay(uint16_t delay_ms)
{
	pas_stop_delay_periods = delay_ms * 10;
}

uint16_t pas_get_cadence_rpm_x10()
{
	uint16_t tmp;
	ET0 = 0; // disable timer0 interrupts
	tmp = pas_period_length;
	ET0 = 1;

	if (tmp > 0)
	{
		return (uint16_t)((6000000ul / PAS_SENSOR_NUM_SIGNALS) / tmp);
	}
	else
	{
		return 0;
	}
}

uint16_t pas_get_pulse_counter()
{
	uint16_t tmp;
	ET0 = 0; // disable timer0 interrupts
	tmp = pas_pulse_counter;
	ET0 = 1;

	return tmp;
}

bool pas_is_pedaling_forwards()
{
	uint16_t period_length;
	uint8_t direction_backward;
	ET0 = 0; // disable timer0 interrupts
	period_length = pas_period_length;
	direction_backward = pas_direction_backward;
	ET0 = 1;

	// atomic read operation, no need to disable timer interrupt
	return period_length > 0 && !direction_backward;
}

bool pas_is_pedaling_backwards()
{
	uint16_t period_length;
	uint8_t direction_backward;
	ET0 = 0; // disable timer0 interrupts
	period_length = pas_period_length;
	direction_backward = pas_direction_backward;
	ET0 = 1;

	return period_length > 0 && direction_backward;
}

void speed_sensor_set_signals_per_rpm(uint8_t num_signals)
{
	speed_ticks_per_rpm = num_signals;
}

bool speed_sensor_is_moving()
{
	uint16_t tmp;
	ET0 = 0; // disable timer0 interrupts
	tmp = speed_ticks_period_length;
	ET0 = 1;

	return tmp > 0;
}

uint16_t speed_sensor_get_rpm_x10()
{
	uint16_t tmp;
	ET0 = 0; // disable timer0 interrupts
	tmp = speed_ticks_period_length;
	ET0 = 1;

	if (tmp > 0)
	{
		return 6000000ul / tmp / speed_ticks_per_rpm;
	}

	return 0;
}

uint16_t torque_sensor_get_nm_x100()
{
	return 0;
}

bool torque_sensor_ok()
{
	return true;
}


int16_t temperature_contr_x100()
{
	const float R1 = 5100.f;
	const float invBeta = 1.f / 3600.f;
	static int32_t adc_contr_x100 = 0;

	if (g_config.use_temperature_sensor & TEMPERATURE_SENSOR_CONTR)
	{
		if (adc_contr_x100 == 0)
		{
			adc_contr_x100 = adc_get_temperature_contr() * 100l;
		}
		else
		{
			adc_contr_x100 = EXPONENTIAL_FILTER(adc_contr_x100, adc_get_temperature_contr() * 100l, 4);
		}

		if (adc_contr_x100 != 0)
		{
			float R = R1 * ((102300.f / (102300.f - adc_contr_x100)) - 1.f);
			return (int16_t)(thermistor_ntc_calculate_temperature(R, invBeta) * 100.f + 0.5f);
		}
	}

	return 0;
}

int16_t temperature_motor_x100()
{
	// Sensor only present in the BBSHD motor
#ifdef BBSHD
	const float R1 = 5100.f;
	const float invBeta = 1.f / 3990.f;

	static int32_t adc_motor_x100 = 0;

	if (g_config.use_temperature_sensor & TEMPERATURE_SENSOR_MOTOR)
	{
		bool first = false;
		if (adc_motor_x100 == 0)
		{
			first = true;
			adc_motor_x100 = adc_get_temperature_motor() * 100l;
		}
		else
		{
			adc_motor_x100 = EXPONENTIAL_FILTER(adc_motor_x100, adc_get_temperature_motor() * 100l, 4);
		}

		if (adc_motor_x100 != 0)
		{
			float R = R1 * ((102300.f / (102300.f - adc_motor_x100)) - 1.f);

			if (first)
			{
				if (R > 1500.f)
				{
					// not likely to be a 1k ptc thermistor, assume 10k ntc
					bbshd_ptc_thermistor = false;
					eventlog_write_data(EVT_DATA_BBSHD_THERMISTOR, 0);
				}
				else
				{
					bbshd_ptc_thermistor = true;
					eventlog_write_data(EVT_DATA_BBSHD_THERMISTOR, 1);
				}
			}
		
			if (bbshd_ptc_thermistor)
			{
				return thermistor_ptc_bbshd_calculate_temperature((int32_t)(R * 100.f + 0.5f));
			}
			else
			{
				return(int16_t)(thermistor_ntc_calculate_temperature(R, invBeta) * 100.f + 0.5f);
			}
		}
	}
#endif

	return 0;
}


bool brake_is_activated()
{
	return !GET_PIN_STATE(PIN_BRAKE);
}

bool shift_sensor_is_activated()
{
	return !GET_PIN_STATE(PIN_SHIFT_SENSOR);
}


#pragma save  
#pragma nooverlay // See SDCC manual about function calls in ISR
void sensors_timer0_isr() // runs every 100us, see timers.c
{
	// WARNING:
	// No 16/32 bit or float computations in ISR (multiply/divide/modulo).
	// Read SDCC compiler manual for more info.

	// Pas
	{
		bool pas1 = GET_PIN_STATE(PIN_PAS1);
		bool pas2 = GET_PIN_STATE(PIN_PAS2);

		if (pas1 && !pas_prev1 /* && pas_period_counter > PAS_SENSOR_MIN_PULSE_MS_X10 */)
		{
			pas_pulse_counter++;

			if (pas_direction_backward != pas2)
			{
				pas_direction_backward = pas2;

				// Reset pas pulse counter if pedal direction is changed,
				// this variable counts the number of pulses since start of pedaling session.
				pas_pulse_counter = 0;
			}
			
			if (pas_period_counter > 0)
			{
				if (pas_period_counter <= pas_stop_delay_periods)
				{
					pas_period_length = pas_period_counter; // save in order to be able to calculate rpm when needed
				}
				else
				{
					pas_period_length = 0;
				}

				pas_period_counter = 0;
			}
		}
		else
		{
			// Do not allow wraparound or computed pedaling cadence will wrong after pedals has been still.
			if (pas_period_counter < 65535)
			{
				pas_period_counter++;
			}
			
			if (pas_period_length > 0 && pas_period_counter > pas_stop_delay_periods)
			{
				pas_period_length = 0;
				pas_pulse_counter = 0;
				pas_direction_backward = false;
			}
		}

		pas_prev1 = pas1;
		pas_prev2 = pas2;
	}


	// Speed sensor
	{
		bool spd = GET_PIN_STATE(PIN_SPEED_SENSOR);

		if (spd && !speed_prev_state && speed_period_counter > SPEED_SENSOR_MIN_PULSE_MS_X10)
		{
			if (speed_period_counter <= SPEED_SENSOR_TIMEOUT_MS_X10)
			{
				speed_ticks_period_length = speed_period_counter;
			}
			else
			{
				speed_ticks_period_length = 0;
			}
				
			speed_period_counter = 0;
		}
		else
		{
			// Do not allow wraparound or computed speed will wrong after bike has been still.
			if (speed_period_counter < 65535)
			{
				speed_period_counter++;
			}
			
			if (speed_ticks_period_length > 0 && speed_period_counter > SPEED_SENSOR_TIMEOUT_MS_X10)
			{
				speed_ticks_period_length = 0;
			}
		}

		speed_prev_state = spd;
	}
	
}
#pragma restore
