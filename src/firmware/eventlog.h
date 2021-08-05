/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _EVENTLOG_H_
#define _EVENTLOG_H_

#include <stdbool.h>
#include <stdint.h>

#define EVT_MSG_MOTOR_INIT_OK				1
#define EVT_MSG_CONFIG_READ					2
#define EVT_MSG_CONFIG_RESET				3
#define EVT_MSG_CONFIG_WRITTEN				4

#define EVT_ERROR_INIT_MOTOR				64
#define EVT_ERROR_CHANGE_TARGET_SPEED		65
#define EVT_ERROR_CHANGE_TARGET_CURRENT		66
#define EVT_ERROR_READ_MOTOR_STATUS			67
#define EVT_ERROR_READ_MOTOR_CURRENT		68
#define EVT_ERROR_READ_MOTOR_VOLTAGE		69

#define EVT_ERROR_CONFIG_READ_EEPROM		70
#define EVT_ERROR_CONFIG_WRITE_EEPROM		71
#define EVT_ERROR_CONFIG_ERASE_EEPROM		72
#define EVT_ERROR_CONFIG_VERSION			73
#define EVT_ERROR_CONFIG_CHECKSUM			74


#define EVT_DATA_TARGET_CURRENT				128
#define EVT_DATA_TARGET_SPEED				129
#define EVT_DATA_MOTOR_STATUS				130
#define EVT_DATA_ASSIST_LEVEL				131
#define EVT_DATA_OPERATION_MODE				132
#define EVT_DATA_WHEEL_SPEED_PPM			133
#define EVT_DATA_LIGHTS						134
#define EVT_DATA_TEMPERATURE				135
#define EVT_DATA_THERMAL_LIMITING			136
#define EVT_DATA_SPEED_LIMITING				137
#define EVT_DATA_MAX_CURRENT_ADC_REQUEST	138
#define EVT_DATA_MAX_CURRENT_ADC_RESPONSE	139


void eventlog_init(bool enabled);

bool eventlog_is_enabled();
void eventlog_set_enabled(bool enabled);

void eventlog_write(uint8_t evt);
void eventlog_write_data(uint8_t evt, int16_t data);


#endif
