/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _ADC_H_
#define _ADC_H_
#include <stdint.h>

void adc_init();
void adc_process();

uint8_t adc_get_throttle();
uint16_t adc_get_torque();

uint16_t adc_get_temperature_contr();
uint16_t adc_get_temperature_motor();

#endif
