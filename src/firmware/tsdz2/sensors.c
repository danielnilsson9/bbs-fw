/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "sensors.h"
#include "intellisense.h"
#include "fwconfig.h"
#include "tsdz2/interrupt.h"
#include "tsdz2/timers.h"
#include "tsdz2/stm8.h"
#include "tsdz2/pins.h"
#include "tsdz2/stm8s/stm8s_tim4.h"

// interrupt runs at 100us interval, see timer4 setup in timers.c

// :TODO: this file contains a lot of duplicated code from bbsx version, try to share code

#define PAS_SENSOR_NUM_SIGNALS			PAS_PULSES_REVOLUTION
#define PAS_SENSOR_MIN_PULSE_MS_X10		50	// 500rpm limit

#define SPEED_SENSOR_MIN_PULSE_MS_X10	500
#define SPEED_SENSOR_TIMEOUT_MS_X10		25000


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

extern void torque_sensor_init();
extern void torque_sensor_process();

void sensors_init()
{
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
	SET_PIN_INPUT_PULLUP(PIN_BRAKE);

	pas_prev1 = GET_PIN_INPUT_STATE(PIN_PAS1);
	pas_prev2 = GET_PIN_INPUT_STATE(PIN_PAS2);

	torque_sensor_init();
	torque_sensor_process();

	timer4_init_sensors();
}

void sensors_process()
{
	torque_sensor_process();
}


void pas_set_stop_delay(uint16_t delay_ms)
{
	pas_stop_delay_periods = delay_ms * 10;
}

uint16_t pas_get_cadence_rpm_x10()
{
	uint16_t tmp;
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupt
	tmp = pas_period_length;
	TIM4->IER |= TIM4_IT_UPDATE;

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
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupts
	tmp = pas_pulse_counter;
	TIM4->IER |= TIM4_IT_UPDATE;

	return tmp;
}

bool pas_is_pedaling_forwards()
{
	uint16_t period_length;
	uint8_t direction_backward;
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupts
	period_length = pas_period_length;
	direction_backward = pas_direction_backward;
	TIM4->IER |= TIM4_IT_UPDATE;

	// atomic read operation, no need to disable timer interrupt
	return period_length > 0 && !direction_backward;
}

bool pas_is_pedaling_backwards()
{
	uint16_t period_length;
	uint8_t direction_backward;
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupts
	period_length = pas_period_length;
	direction_backward = pas_direction_backward;
	TIM4->IER |= TIM4_IT_UPDATE;

	return (period_length > 0) && direction_backward;
}

void speed_sensor_set_signals_per_rpm(uint8_t num_signals)
{
	speed_ticks_per_rpm = num_signals;
}

bool speed_sensor_is_moving()
{
	uint16_t tmp;
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupts
	tmp = speed_ticks_period_length;
	TIM4->IER |= TIM4_IT_UPDATE;

	return tmp > 0;
}

uint16_t speed_sensor_get_rpm_x10()
{
	uint16_t tmp;
	TIM4->IER &= ~TIM4_IT_UPDATE; // disable timer4 interrupts
	tmp = speed_ticks_period_length;
	TIM4->IER |= TIM4_IT_UPDATE;

	if (tmp > 0)
	{
		return 6000000ul / tmp / speed_ticks_per_rpm;
	}

	return 0;
}


int16_t temperature_contr_x100()
{
	return 0; // n/a
}

int16_t temperature_motor_x100()
{
	return 0; // n/a
}


bool brake_is_activated()
{
	return !GET_PIN_INPUT_STATE(PIN_BRAKE);
}

bool shift_sensor_is_activated()
{
	return false; // n/a
}


void isr_timer4_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF)
{
	// clear interrupt bit
	TIM4->SR1 &= (uint8_t)(~TIM4_IT_UPDATE);

	// Pas
	{
		bool pas1 = GET_PIN_INPUT_STATE(PIN_PAS1);
		bool pas2 = GET_PIN_INPUT_STATE(PIN_PAS2);

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
		bool spd = GET_PIN_INPUT_STATE(PIN_SPEED_SENSOR);

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
