/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "sensors.h"
#include "pins.h"
#include "stc15.h"
#include "uart.h"
#include "system.h"
#include "adc.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define TIMER4_RELOAD	((65536 - CPU_FREQ / 10000) + 1)		// every 100us

#define NUM_SIGNALS		24


static volatile uint16_t pas_pulse_counter;
static volatile bool pas_direction_backward;
static volatile uint8_t pas_rpm;
static __xdata uint16_t pas_period_counter;
static __xdata bool pas_prev1;
static __xdata bool pas_prev2;
static __xdata uint16_t pas_stop_delay_periods;

static volatile uint16_t speed_ticks_minute_x10;
static __xdata uint16_t speed_period_counter;
static __xdata bool speed_prev_state;
static __xdata uint8_t speed_ticks_per_rpm;


void sensors_init()
{
	pas_period_counter = 0;
	pas_pulse_counter = 0;
	pas_direction_backward = false;
	pas_rpm = 0;
	pas_stop_delay_periods = 1500;
	speed_period_counter = 0;
	speed_ticks_minute_x10 = 0;
	speed_prev_state = false;
	speed_ticks_per_rpm = 1;

	// pins do not have external interrupt, use timer 4 to evaluate state frequently
	SET_PIN_INPUT(PIN_PAS1);
	SET_PIN_INPUT(PIN_PAS2);
	SET_PIN_INPUT(PIN_SPEED_SENSOR);
	SET_PIN_INPUT(PIN_BRAKE);
	SET_PIN_INPUT(PIN_GEAR_SENSOR);

	pas_prev1 = GET_PIN_STATE(PIN_PAS1);
	pas_prev2 = GET_PIN_STATE(PIN_PAS2);

	CLEAR_BIT(T4T3M, 6); // use as timer
	SET_BIT(T4T3M, 5); // Run at CPU_FREQ

	T4H = TIMER4_RELOAD >> 8;
	T4L = TIMER4_RELOAD & 0x0ff;

	SET_BIT(IE2, 6); // Enable interrupts timer 4
	SET_BIT(T4T3M, 7); // start timer 4
}


void pas_set_stop_delay(uint16_t delay_ms)
{
	pas_stop_delay_periods = delay_ms * 10;
}

uint8_t pas_get_cadence_rpm()
{
	return pas_rpm;
}

uint16_t pas_get_pulse_counter()
{
	return pas_pulse_counter;
}

bool pas_is_pedaling_forwards()
{
	return pas_rpm > 0 && !pas_direction_backward;
}

bool pas_is_pedaling_backwards()
{
	return pas_rpm > 0 && pas_direction_backward;
}

void speed_sensor_set_signals_per_rpm(uint8_t num_signals)
{
	speed_ticks_per_rpm = num_signals;
}

bool speed_sensor_is_moving()
{
	return speed_ticks_minute_x10 > 0;
}

uint16_t speed_sensor_get_rpm_x10()
{
	return speed_ticks_minute_x10 / speed_ticks_per_rpm;
}

int8_t temperature_read()
{
	uint8_t adc = adc_get_temperature();

	if (adc != 0)
	{
		// :TODO: Measure and calculate beta value for range 25 - 80 degrees
		const float invBeta = 1.f / 3600.f;
		const float invT0 = 1.f / 298.15f;

		float R = 5100.f * ((255.f / (255.f - adc)) - 1.f);

		float K = 1.f / (invT0 + invBeta * (logf(R / 10000.f)));
		float C = K - 273.15f;

		return (int8_t)(C + 0.5f);
	}

	return 0.f;
}


bool brake_is_activated()
{
	return !GET_PIN_STATE(PIN_BRAKE);
}

bool gear_sensor_is_activated()
{
	return !GET_PIN_STATE(PIN_GEAR_SENSOR);
}


INTERRUPT(isr_timer4, IRQ_TIMER4)
{
	// pas
	{
		bool pas1 = GET_PIN_STATE(PIN_PAS1);
		bool pas2 = GET_PIN_STATE(PIN_PAS2);

		if (pas1 && !pas_prev1)
		{
			++pas_pulse_counter;
			pas_direction_backward = pas2;

			if (pas_period_counter > 0)
			{
				pas_rpm = (uint8_t)((600000UL / NUM_SIGNALS) / pas_period_counter);
				pas_period_counter = 0;
			}
		}
		else
		{
			++pas_period_counter;

			if (pas_rpm > 0 && pas_period_counter > pas_stop_delay_periods)
			{
				pas_rpm = 0;
				pas_pulse_counter = 0;
				pas_direction_backward = false;
			}
		}

		pas_prev1 = pas1;
		pas_prev2 = pas2;
	}


	// speed sensor
	{
		bool spd = GET_PIN_STATE(PIN_SPEED_SENSOR);

		if (spd && !speed_prev_state)
		{
			if (speed_period_counter > 0)
			{
				speed_ticks_minute_x10 = 6000000UL / speed_period_counter;
				speed_period_counter = 0;
			}
		}
		else
		{
			++speed_period_counter;

			if (speed_ticks_minute_x10 > 0 && speed_period_counter > 50000)
			{
				speed_ticks_minute_x10 = 0;
			}
		}

		speed_prev_state = spd;
	}
	
}


