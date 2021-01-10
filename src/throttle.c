/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#include "throttle.h"
#include "stc15.h"
#include "pins.h"
#include "system.h"


void throttle_init()
{
	// Setup pin 3 as adc input
	SET_PIN_INPUT(PIN_THROTTLE);
	SET_PIN_LOW(PIN_THROTTLE);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_THROTTLE));

	// Arrange adc result for 8bit reading
	CLEAR_BIT(PCON2, 5);

	SET_BIT(ADC_CONTR, 7);	// enable adc power
}


void throttle_set_min_voltage(uint8_t volt_x1000)
{

}

void throttle_set_max_voltage(uint8_t volt_x1000)
{

}

uint8_t throttle_read()
{
	ADC_RES = 0;

	// Sample ADC on pin 3.
	ADC_CONTR = (1 << 7) | (1 << 3) | GET_PIN_NUM(PIN_THROTTLE);

	// as per specification
	NOP();
	NOP();
	NOP();
	NOP();

	while (!IS_BIT_SET(ADC_CONTR, 4));
	CLEAR_BIT(ADC_CONTR, 4);

	return ADC_RES;
}

