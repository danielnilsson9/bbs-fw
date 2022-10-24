/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */
#include <stdint.h>
#include "adc.h"
#include "tsdz2/stm8.h"
#include "tsdz2/pins.h"
#include "tsdz2/stm8s/stm8s_adc1.h"


void adc_init()
{
	SET_PIN_INPUT(PIN_BATTERY_CURRENT);
	SET_PIN_INPUT(PIN_BATTERY_VOLTAGE);
	SET_PIN_INPUT(PIN_THROTTLE);
	SET_PIN_INPUT(PIN_TORQUE_SENSOR);

	// single mode
	ADC1->CR1 &= (uint8_t)(~ADC1_CR1_CONT);
	ADC1->CR2 |= ADC1_ALIGN_RIGHT;

	// channel (none)
	ADC1->CSR |= 0x00;

	// prescaler
	ADC1->CR1 |= (uint8_t)(ADC1_PRESSEL_FCPU_D2);

	// no trigger
	ADC1->CR2 &= (uint8_t)(~ADC1_CR2_EXTTRIG);
	ADC1->CR2 |= (uint8_t)(ADC1_EXTTRIG_TIM);

	// schmittrig disable all
	ADC1->TDRL |= (uint8_t)0xFF;
	ADC1->TDRH |= (uint8_t)0xFF;

	// enable scan mode
	ADC1->CR2 |= ADC1_CR2_SCAN;
	
	// Enable the ADC1 peripheral
	ADC1->CR1 |= ADC1_CR1_ADON;
}

void adc_process()
{
	// nothing to do, adc reading is triggered by interrupt code in motor.c
}


uint8_t adc_get_throttle()
{
	// 8bit resolution expected
	return (uint8_t)(((ADC1->DB7RH << 8) | ADC1->DB7RL) >> 2);
}

uint16_t adc_get_torque()
{
	// 10 bit resolution
	return ((ADC1->DB4RH << 8) | ADC1->DB4RL);
}

uint16_t adc_get_temperature_contr()
{
	return 0;
}

uint16_t adc_get_temperature_motor()
{
	return 0;
}
