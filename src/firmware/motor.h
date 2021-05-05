/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stc15.h"
#include <stdint.h>

void motor_init(__xdata uint16_t max_current_mA, __xdata uint8_t lvc_V);
void motor_process();

void motor_enable();
void motor_disable();

__xdata uint16_t motor_status();

void motor_set_target_speed(uint8_t value);
void motor_set_target_current(uint8_t percent);

__xdata uint16_t motor_get_battery_lvc_x10();
__xdata uint16_t motor_get_battery_current_x10();
__xdata uint16_t motor_get_battery_voltage_x10();

#endif
