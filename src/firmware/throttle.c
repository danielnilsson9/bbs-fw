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
#include "util.h"

#include <stdbool.h>


static uint16_t __xdata min_voltage_x1000;
static uint16_t __xdata max_voltage_x1000;
static uint8_t __xdata start_percent;

static bool __xdata throttle_ok;


void throttle_init(uint16_t min_mv, uint16_t max_mv)
{
	min_voltage_x1000 = min_mv;
	max_voltage_x1000 = max_mv;
	start_percent = 0;
	throttle_ok = false;

	// Setup pin 3 as adc input
	SET_PIN_INPUT(PIN_THROTTLE);
	SET_PIN_LOW(PIN_THROTTLE);
	SET_BIT(P1ASF, GET_PIN_NUM(PIN_THROTTLE));

	// Arrange adc result for 8bit reading
	CLEAR_BIT(PCON2, 5);

	SET_BIT(ADC_CONTR, 7);	// enable adc power
}


void throttle_set_start_percent(uint8_t value)
{
	start_percent = value;
}

uint8_t throttle_read()
{
	ADC_RES = 0;

	// Sample ADC on pin 3.
	ADC_CONTR = (uint8_t)((1 << 7) | (1 << 3) | (GET_PIN_NUM(PIN_THROTTLE) & 0x07));

	// as per specification
	NOP();
	NOP();
	NOP();
	NOP();

	while (!IS_BIT_SET(ADC_CONTR, 4));
	CLEAR_BIT(ADC_CONTR, 4);

	uint16_t volt_x1000 = (uint16_t)((ADC_RES * 4300UL) / 256);

	if (volt_x1000 < min_voltage_x1000)
	{
		throttle_ok = true;
		return 0;
	}

	if (volt_x1000 > max_voltage_x1000)
	{
		volt_x1000 = max_voltage_x1000;
	}

	return (uint8_t)map(volt_x1000, min_voltage_x1000, max_voltage_x1000, start_percent, 100);
}

