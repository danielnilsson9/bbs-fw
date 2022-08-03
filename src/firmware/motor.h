/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stc15.h"
#include <stdint.h>

#define MOTOR_ERROR_LVC				0x0800
#define MOTOR_ERROR_HALL_SENSOR		0x2000
#define MOTOR_ERROR_CURRENT_SENSE	0x0004

void motor_init(uint16_t max_current_mA, uint8_t lvc_V);
void motor_process();

void motor_enable();
void motor_disable();

uint16_t motor_status();

void motor_set_target_speed(uint8_t value);
void motor_set_target_current(uint8_t percent);

uint16_t motor_get_battery_lvc_x10();
uint16_t motor_get_battery_current_x10();
uint16_t motor_get_battery_voltage_x10();

#endif
