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

void motor_set_target_speed(uint8_t value);
void motor_set_target_current(uint8_t percent);

void motor_process();

#endif
