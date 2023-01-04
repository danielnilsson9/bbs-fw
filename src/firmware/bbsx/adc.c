/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "adc.h"
#include "bbsx/stc15.h"
#include "bbsx/pins.h"


static uint8_t next_channel;
static uint8_t no_adc_reading_counter;

static uint8_t throttle_value;
static uint16_t temperature_contr_value;
static uint16_t temperature_motor_value;


void adc_init()
{
	// Setup pin voltage as high impedance input even though it is not used
	SET_PIN_INPUT(PIN_VOLTAGE);

	// Setup pin throttle as adc input
	SET_PIN_INPUT(PIN_THROTTLE);
	SET_PIN_LOW(PIN_THROTTLE);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_THROTTLE));

	// Setup pin controller temperature pin as adc input
	SET_PIN_INPUT(PIN_TEMPERATURE_CONTR);
	SET_PIN_LOW(PIN_TEMPERATURE_CONTR);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_TEMPERATURE_CONTR));

#ifdef BBSHD
	// Setup pin motor temperature pin as adc input
	SET_PIN_INPUT(PIN_TEMPERATURE_MOTOR);
	SET_PIN_LOW(PIN_TEMPERATURE_MOTOR);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_TEMPERATURE_MOTOR));
#endif

	ADC_RES = 0;
	ADC_RESL = 0;

	// Arrange adc result for 8bit reading
	CLEAR_BIT(PCON2, 5);

	ADC_CONTR = (uint8_t)((1 << 7));

	no_adc_reading_counter = 0;
	throttle_value = 0;
	temperature_contr_value = 0;
	temperature_motor_value = 0;
	next_channel = GET_PIN_NUM(PIN_THROTTLE);


	// throttle is read during init since a valid value must be
	// needs to be available for fir throttle_process to not mess
	// up safeguard logic

	// enable adc power and read throttle
	ADC_CONTR = (uint8_t)((1 << 7) | (1 << 3) | next_channel);

	// wait for throttle reading and process
	while (!IS_BIT_SET(ADC_CONTR, 4));
	adc_process();
}

void adc_process()
{
	// adc reading available
	if (IS_BIT_SET(ADC_CONTR, 4))
	{
		no_adc_reading_counter = 0;

		ADC_CONTR = (uint8_t)((1 << 7)); // Clear ADC_FLAG 

		switch (next_channel)
		{
		case GET_PIN_NUM(PIN_THROTTLE):
		{
			throttle_value = ADC_RES;
			next_channel = GET_PIN_NUM(PIN_TEMPERATURE_CONTR);
			break;
		}
		case GET_PIN_NUM(PIN_TEMPERATURE_CONTR):
		{
			temperature_contr_value = (((uint16_t)ADC_RES) << 2) | ADC_RESL;
#ifdef BBSHD
			next_channel = GET_PIN_NUM(PIN_TEMPERATURE_MOTOR);
#else
			next_channel = GET_PIN_NUM(PIN_THROTTLE);
#endif
			break;
		}
#ifdef BBSHD
		case GET_PIN_NUM(PIN_TEMPERATURE_MOTOR):
		{
			temperature_motor_value = (((uint16_t)ADC_RES) << 2) | ADC_RESL;
			next_channel = GET_PIN_NUM(PIN_THROTTLE);
			break;
		}
#endif
		}
	}
	else if (++no_adc_reading_counter == 0)
	{
		// reinitialize adc
		ADC_RES = 0;
		ADC_CONTR = (uint8_t)(1 << 7);
		no_adc_reading_counter = 0;
		throttle_value = 0;
		temperature_motor_value = 0;
		temperature_contr_value = 0;
		next_channel = GET_PIN_NUM(PIN_THROTTLE);
	}
	else
	{
		return;
	}

	// start next reading
	ADC_RES = 0;
	ADC_CONTR = (uint8_t)((1 << 7) | (1 << 3) | next_channel);
}


uint8_t adc_get_throttle()
{
	return throttle_value;
}

uint16_t adc_get_torque()
{
	return 0;
}

uint16_t adc_get_temperature_contr()
{
	return temperature_contr_value;
}

uint16_t adc_get_temperature_motor()
{
	return temperature_motor_value;
}
