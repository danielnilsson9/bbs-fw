/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "pas.h"
#include "pins.h"
#include "stc15.h"
#include "uart.h"
#include "system.h"

#include <stdint.h>
#include <stdbool.h>

#define TIMER4_RELOAD	((65536 - CPU_FREQ / 10000) + 1)		// every 100us

#define NUM_SIGNALS		24


volatile uint16_t period_counter = 0;
volatile bool prev_pas1 = 0;
volatile bool prev_pas2 = 0;
volatile bool direction_backward = 0;

volatile uint8_t rpm = 0;


void pas_init()
{
	// pins do not have external interrupt, use timer 4 to evaluate state frequently
	SET_PIN_INPUT(PIN_PAS1);
	SET_PIN_INPUT(PIN_PAS2);

	prev_pas1 = GET_PIN_STATE(PIN_PAS1);
	prev_pas2 = GET_PIN_STATE(PIN_PAS2);

	CLEAR_BIT(T4T3M, 6); // use as timer
	SET_BIT(T4T3M, 5); // Run at CPU_FREQ

	T4H = TIMER4_RELOAD >> 8;
	T4L = TIMER4_RELOAD;

	SET_BIT(IE2, 6); // Enable interrupts timer 4
	SET_BIT(T4T3M, 7); // start timer 4
}

uint8_t pas_get_rpm()
{
	return rpm;
}

bool pas_is_pedaling_forwards()
{
	return rpm > 0 && !direction_backward;
}

bool pas_is_pedaling_backwards()
{
	return rpm > 0 && direction_backward;
}

INTERRUPT(isr_timer4, IRQ_TIMER4)
{
	bool pas1 = GET_PIN_STATE(PIN_PAS1);
	bool pas2 = GET_PIN_STATE(PIN_PAS2);

	period_counter++;

	if (pas1 && !prev_pas1)
	{
		direction_backward = pas2;

		rpm = (uint8_t)((600000UL / NUM_SIGNALS) / period_counter);
		period_counter = 0;
	}
	else
	{
		if (rpm > 0 && period_counter > 1000)
		{
			rpm = 0;
			direction_backward = false;
		}
	}

	prev_pas1 = pas1;
	prev_pas2 = pas2;
}


