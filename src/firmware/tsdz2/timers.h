/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _TSDZ2_TIMERS_H_
#define _TSDZ2_TIMERS_H_


void timer1_init_motor_pwm();
void timer2_init_torque_sensor_pwm();
void timer3_init_system();
void timer4_init_sensors();

#endif
