/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#include "adc.h"
#include "stc15.h"
#include "pins.h"


static __xdata uint8_t next_channel;
static __xdata uint8_t no_adc_reading_counter;

static __xdata uint8_t throttle_value;
static __xdata uint8_t temperature_value;


void adc_init()
{
	// Setup pin throttle as adc input
	SET_PIN_INPUT(PIN_THROTTLE);
	SET_PIN_LOW(PIN_THROTTLE);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_THROTTLE));

	// Setup pin temperature pin as adc input
	SET_PIN_INPUT(PIN_TEMPERATURE);
	SET_PIN_LOW(PIN_TEMPERATURE);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_TEMPERATURE));

	ADC_RES = 0;
	ADC_RESL = 0;

	// Arrange adc result for 8bit reading
	CLEAR_BIT(PCON2, 5);

	// enable adc power
	ADC_CONTR = (uint8_t)((1 << 7));

	no_adc_reading_counter = 0;
	throttle_value = 0;
	temperature_value = 0;
	next_channel = GET_PIN_NUM(PIN_THROTTLE);
}

void adc_process()
{
	// adc reading available
	if (IS_BIT_SET(ADC_CONTR, 4))
	{
		switch (next_channel)
		{
		case GET_PIN_NUM(PIN_THROTTLE):
		{
			throttle_value = ADC_RES;
			next_channel = GET_PIN_NUM(PIN_TEMPERATURE);
			break;
		}
		case GET_PIN_NUM(PIN_TEMPERATURE):
		{
			temperature_value = ADC_RES;
			next_channel = GET_PIN_NUM(PIN_THROTTLE);
			break;
		}}
	}
	else if (++no_adc_reading_counter > 8)
	{
		// reinitialize adc
		ADC_RES = 0;
		ADC_RESL = 0;
		ADC_CONTR = (uint8_t)(1 << 7);
		no_adc_reading_counter = 0;
		throttle_value = 0;
		temperature_value = 0;
		next_channel = GET_PIN_NUM(PIN_THROTTLE);
	}
	else
	{
		return;
	}

	// start next reading
	ADC_CONTR = (uint8_t)((1 << 7) | (1 << 3) | next_channel);
}


uint8_t adc_get_throttle()
{
	return throttle_value;
}

uint8_t adc_get_temperature()
{
	return temperature_value;
}
