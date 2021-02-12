/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef  _PINS_H_
#define _PINS_H_

// PORT, PIN

#define PIN_MOTOR_POWER_ENABLE			2, 0
#define PIN_MOTOR_CONTROL_ENABLE		2, 1
#define PIN_MOTOR_EXTRA					4, 4
#define PIN_MOTOR_RX					1, 0
#define PIN_MOTOR_TX					1, 1

#define PIN_VOLTAGE						1, 6
#define PIN_TEMPERATURE					1, 7

#define PIN_PAS1						4, 5
#define PIN_PAS2						4, 6

#define PIN_HALL_U						5, 0
#define PIN_HALL_V						3, 4
#define PIN_HALL_W						0, 6

#define PIN_SPEED_SENSOR				2, 2
#define PIN_BRAKE						2, 4
#define PIN_THROTTLE					1, 3

#define PIN_EXTERNAL_RX					3, 0
#define PIN_EXTERNAL_TX					3, 1

#endif
