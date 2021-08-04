/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _ADC_H_
#define _ADC_H_
#include <stdint.h>


void adc_init();
void adc_process();

uint8_t adc_get_throttle();
uint8_t adc_get_temperature();

#endif
