/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <stdint.h>


void motor_init(uint16_t max_current_mA, uint8_t lvc_V);

void motor_enable();
void motor_disable();

void motor_set_target_current(uint16_t target_mA);

void motor_process();

#endif
