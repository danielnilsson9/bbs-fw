/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "stc15.h"
#include "interrupt.h"

#include <stdint.h>
#include <stdbool.h>

void sensors_init();

uint8_t pas_get_cadence_rpm();
uint16_t pas_get_pulse_counter();
bool pas_is_pedaling_forwards();
bool pas_is_pedaling_backwards();

void speed_sensor_set_signals_per_rpm(uint8_t num_signals);
bool speed_sensor_is_moving();
uint16_t speed_sensor_get_rpm_x10();

uint8_t temperature_read();

bool brake_is_activated();


INTERRUPT(isr_timer4, IRQ_TIMER4);

#endif
