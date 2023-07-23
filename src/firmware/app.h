/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _APP_H_
#define _APP_H_

#include "intellisense.h"
#include <stdint.h>
#include <stdbool.h>


#define ASSIST_0		0x00
#define ASSIST_1		0x01
#define ASSIST_2		0x02
#define ASSIST_3		0x03
#define ASSIST_4		0x04
#define ASSIST_5		0x05
#define ASSIST_6		0x06
#define ASSIST_7		0x07
#define ASSIST_8		0x08
#define ASSIST_9		0x09
#define ASSIST_PUSH		0x0A

#define OPERATION_MODE_DEFAULT	0x00
#define OPERATION_MODE_SPORT	0x01

// Matches status codes used by Bafang
#define STATUS_NORMAL						0x01
#define STATUS_BRAKING						0x03

#define STATUS_ERROR_THROTTLE_HIGH			0x04
#define STATUS_ERROR_THROTTLE				0x05
#define STATUS_ERROR_LVC					0x06
#define STATUS_ERROR_HIGH_VOLTAGE			0x07 // not implemented
#define STATUS_ERROR_HALL_SENSOR			0x08
#define STATUS_ERROR_PHASE_LINE				0x09
#define STATUS_ERROR_CONTROLLER_OVER_TEMP	0x10
#define STATUS_ERROR_MOTOR_OVER_TEMP		0x11
#define STATUS_ERROR_CURRENT_SENSE			0x12
#define STATUS_ERROR_BATTERY_TEMP_SENSOR	0x13 // n/a
#define STATUS_ERROR_MOTOR_TEMP_SENSOR		0x14 // not implemented
#define STATUS_ERROR_CONTROLLER_TEMP_SENSOR 0x15 // not implemented
#define STATUS_ERROR_SPEED_SENSOR			0x21 // not implemented
#define STATUS_ERROR_BMS_COMMUNICATION		0x22 // n/a
#define STATUS_ERROR_HEAD_LIGHT				0x23 // not implemented
#define STATUS_ERROR_HEAD_LIGHT_SENSOR		0x24 // not implemented
#define STATUS_ERROR_TORQUE_SENSOR			0x25
#define STATUS_ERROR_TORQUE_SPEED			0x26 // n/a
#define STATUS_ERROR_COMMUNICATION			0x30 // n/a


void app_init();

void app_process();

void app_set_assist_level(uint8_t level);
void app_set_lights(bool on);
void app_set_operation_mode(uint8_t mode);
void app_set_wheel_max_speed_rpm(uint16_t value);

uint8_t app_get_assist_level();
uint8_t app_get_status_code();
uint8_t app_get_temperature();

#endif
