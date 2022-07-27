/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
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

#define TIMER4_RELOAD	((65535 - CPU_FREQ / 10000) + 1)		// every 100us

#define NUM_SIGNALS		24


static volatile uint16_t pas_pulse_counter;
static volatile bool pas_direction_backward;
static volatile uint16_t pas_period_length;	// pulse length counted in interrupt frequency (100us)
static __xdata uint16_t pas_period_counter;
static __xdata bool pas_prev1;
static __xdata bool pas_prev2;
static __xdata uint16_t pas_stop_delay_periods;

static volatile uint16_t speed_ticks_period_length; // pulse length counted in interrupt frequency (100us)
static __xdata uint16_t speed_period_counter;
static __xdata bool speed_prev_state;
static __xdata uint8_t speed_ticks_per_rpm;


void sensors_init()
{
	pas_period_counter = 0;
	pas_pulse_counter = 0;
	pas_direction_backward = false;
	pas_period_length = 0;
	//pas_rpm = 0;
	pas_stop_delay_periods = 1500;
	speed_period_counter = 0;
	speed_ticks_period_length = 0;
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
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = pas_period_length;
	SET_BIT(IE2, 6);

	if (tmp > 0)
	{
		return (uint8_t)((600000UL / NUM_SIGNALS) / tmp);
	}
	else
	{
		return 0;
	}
}

uint16_t pas_get_pulse_counter()
{
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = pas_pulse_counter;
	SET_BIT(IE2, 6);

	return tmp;
}

bool pas_is_pedaling_forwards()
{
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = pas_period_length;
	SET_BIT(IE2, 6);

	// atomic read operation, no need to disable timer interrupt
	return tmp > 0 && !pas_direction_backward;
}

bool pas_is_pedaling_backwards()
{
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = pas_period_length;
	SET_BIT(IE2, 6);

	// atomic read operation, no need to disable timer interrupt
	return tmp > 0 && pas_direction_backward;
}

void speed_sensor_set_signals_per_rpm(uint8_t num_signals)
{
	speed_ticks_per_rpm = num_signals;
}

bool speed_sensor_is_moving()
{
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = speed_ticks_period_length;
	SET_BIT(IE2, 6);

	return tmp > 0;
}

uint16_t speed_sensor_get_rpm_x10()
{
	uint16_t tmp;
	CLEAR_BIT(IE2, 6); // disable timer 4 interrupts
	tmp = speed_ticks_period_length;
	SET_BIT(IE2, 6);

	if (tmp > 0)
	{
		return 6000000UL / tmp / speed_ticks_per_rpm;
	}

	return 0;
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

	return 0;
}


bool brake_is_activated()
{
	return !GET_PIN_STATE(PIN_BRAKE);
}

bool gear_sensor_is_activated()
{
	return !GET_PIN_STATE(PIN_GEAR_SENSOR);
}


INTERRUPT_USING(isr_timer4, IRQ_TIMER4, 2)
{
	// WARNING:
	// No 16/32 bit or float computations in ISR (multiply/divide/modulo).
	// Read SDCC compiler manual for more info.

	// pas
	{
		bool pas1 = GET_PIN_STATE(PIN_PAS1);
		bool pas2 = GET_PIN_STATE(PIN_PAS2);

		if (pas1 && !pas_prev1)
		{
			pas_pulse_counter++;
			pas_direction_backward = pas2;

			if (pas_period_counter > 0)
			{
				pas_period_length = pas_period_counter; // save in order to be able to calculate rpm when needed
				pas_period_counter = 0;
			}
		}
		else
		{
			pas_period_counter++;

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


	// speed sensor
	{
		bool spd = GET_PIN_STATE(PIN_SPEED_SENSOR);

		if (spd && !speed_prev_state)
		{
			if (speed_period_counter > 0)
			{
				speed_ticks_period_length = speed_period_counter;
				speed_period_counter = 0;
			}
		}
		else
		{
			++speed_period_counter;

			if (speed_ticks_period_length > 0 && speed_period_counter > 25000)
			{
				speed_ticks_period_length = 0;
			}
		}

		speed_prev_state = spd;
	}
	
}
