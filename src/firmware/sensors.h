/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "intellisense.h"

#include <stdint.h>
#include <stdbool.h>

void sensors_init();
void sensors_process();

void pas_set_stop_delay(uint16_t delay_ms);
uint16_t pas_get_cadence_rpm_x10();
uint16_t pas_get_pulse_counter();
bool pas_is_pedaling_forwards();
bool pas_is_pedaling_backwards();

void speed_sensor_set_signals_per_rpm(uint8_t num_signals);
bool speed_sensor_is_moving();
uint16_t speed_sensor_get_rpm_x10();

uint16_t torque_sensor_get_nm_x100();
bool torque_sensor_ok();

int16_t temperature_contr_x100();
int16_t temperature_motor_x100();

bool brake_is_activated();
bool shift_sensor_is_activated();

#endif
