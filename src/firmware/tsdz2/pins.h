/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */
#ifndef _TSDZ2_PINS_H_
#define _TSDZ2_PINS_H_

#include "tsdz2/cpu.h"
#include "tsdz2/stm8s/stm8s.h"
#include "tsdz2/stm8s/stm8s_gpio.h"

#define PIN_HALL_SENSOR_A			GPIOE, GPIO_PIN_5
#define PIN_HALL_SENSOR_B			GPIOD, GPIO_PIN_2
#define PIN_HALL_SENSOR_C			GPIOC, GPIO_PIN_5

#define PIN_PWM_PHASE_A_LOW			GPIOB, GPIO_PIN_2
#define PIN_PWM_PHASE_A_HIGH		GPIOC, GPIO_PIN_3

#define PIN_PWM_PHASE_B_LOW			GPIOB, GPIO_PIN_1
#define PIN_PWM_PHASE_B_HIGH		GPIOC, GPIO_PIN_2

#define PIN_PWM_PHASE_C_LOW			GPIOB, GPIO_PIN_0
#define PIN_PWM_PHASE_C_HIGH		GPIOC, GPIO_PIN_1

#define PIN_BATTERY_CURRENT			GPIOB, GPIO_PIN_5
#define PIN_BATTERY_VOLTAGE			GPIOB, GPIO_PIN_6

#define PIN_PAS1					GPIOD, GPIO_PIN_7
#define PIN_PAS2					GPIOE, GPIO_PIN_0
#define PIN_SPEED_SENSOR			GPIOA, GPIO_PIN_1
#define PIN_BRAKE					GPIOC, GPIO_PIN_6
#define PIN_THROTTLE				GPIOB, GPIO_PIN_7
#define PIN_LIGHTS					GPIOD, GPIO_PIN_4

#define PIN_TORQUE_SENSOR			GPIOB, GPIO_PIN_3
#define PIN_TORQUE_SENSOR_EXC		GPIOD, GPIO_PIN_3

#define PIN_EXTERNAL_RX				GPIOD, GPIO_PIN_6
#define PIN_EXTERNAL_TX				GPIOD, GPIO_PIN_5

#endif
