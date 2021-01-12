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

#include <stdint.h>
#include <stdbool.h>

#define TIMER4_RELOAD	((65536 - CPU_FREQ / 10000) + 1)		// every 100us

#define NUM_SIGNALS		24


static volatile uint16_t pas_period_counter = 0;
static volatile uint16_t pas_pulse_counter = 0;
static volatile bool direction_backward = 0;
static volatile uint8_t pas_rpm = 0;
static bool pas_prev1 = false;
static bool pas_prev2 = false;

static volatile uint16_t speed_period_counter = 0;
static volatile uint16_t speed_ticks_minute = 0;
bool speed_prev_state = false;


void sensors_init()
{
	// pins do not have external interrupt, use timer 4 to evaluate state frequently
	SET_PIN_INPUT(PIN_PAS1);
	SET_PIN_INPUT(PIN_PAS2);
	SET_PIN_INPUT(PIN_SPEED_SENSOR);
	SET_PIN_INPUT(PIN_BRAKE);

	pas_prev1 = GET_PIN_STATE(PIN_PAS1);
	pas_prev2 = GET_PIN_STATE(PIN_PAS2);



	CLEAR_BIT(T4T3M, 6); // use as timer
	SET_BIT(T4T3M, 5); // Run at CPU_FREQ

	T4H = TIMER4_RELOAD >> 8;
	T4L = TIMER4_RELOAD;

	SET_BIT(IE2, 6); // Enable interrupts timer 4
	SET_BIT(T4T3M, 7); // start timer 4
}


uint8_t pas_get_cadence_rpm()
{
	return pas_rpm;
}

uint8_t pas_get_pulse_counter()
{
	return pas_pulse_counter;
}

bool pas_is_pedaling_forwards()
{
	return pas_rpm > 0 && !direction_backward;
}

bool pas_is_pedaling_backwards()
{
	return pas_rpm > 0 && direction_backward;
}


bool speed_sensor_is_moving()
{
	return speed_ticks_minute > 0;
}

uint16_t speed_sensor_get_ticks_per_minute()
{
	return speed_ticks_minute;
}

uint8_t temperature_read()
{
	// :TODO:
	return 0;
}


bool brake_is_activated()
{
	return !GET_PIN_STATE(PIN_BRAKE);
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
			direction_backward = pas2;

			if (pas_period_counter > 0)
			{
				pas_rpm = (uint8_t)((600000UL / NUM_SIGNALS) / pas_period_counter);
				pas_period_counter = 0;
			}
		}
		else
		{
			++pas_period_counter;

			if (pas_rpm > 0 && pas_period_counter > 1500)
			{
				pas_rpm = 0;
				pas_pulse_counter = 0;
				direction_backward = false;
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
				speed_ticks_minute = 600000UL / speed_period_counter;
				speed_period_counter = 0;
			}
		}
		else
		{
			++speed_period_counter;

			if (speed_ticks_minute > 0 && speed_period_counter > 50000)
			{
				speed_ticks_minute = 0;
			}
		}

		speed_prev_state = spd;
	}
	
}


