/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _EVENTLOG_H_
#define _EVENTLOG_H_

#include "intellisense.h"

#include <stdbool.h>
#include <stdint.h>

#define EVT_MSG_MOTOR_INIT_OK				1
#define EVT_MSG_CONFIG_READ_DONE			2
#define EVT_MSG_CONFIG_RESET				3
#define EVT_MSG_CONFIG_WRITE_DONE			4
#define EVT_MSG_CONFIG_READ_BEGIN			5
#define EVT_MSG_CONFIG_WRITE_BEGIN			6
#define EVT_MSG_PSTATE_READ_BEGIN			7
#define EVT_MSG_PSTATE_READ_DONE			8
#define EVT_MSG_PSTATE_WRITE_BEGIN			9
#define EVT_MSG_PSTATE_WRITE_DONE			10


#define EVT_ERROR_INIT_MOTOR				64
#define EVT_ERROR_CHANGE_TARGET_SPEED		65
#define EVT_ERROR_CHANGE_TARGET_CURRENT		66
#define EVT_ERROR_READ_MOTOR_STATUS			67
#define EVT_ERROR_READ_MOTOR_CURRENT		68
#define EVT_ERROR_READ_MOTOR_VOLTAGE		69

#define EVT_ERROR_EEPROM_READ				70
#define EVT_ERROR_EEPROM_WRITE				71
#define EVT_ERROR_EEPROM_ERASE				72
#define EVT_ERROR_EEPROM_VERIFY_VERSION		73
#define EVT_ERROR_EEPROM_VERIFY_CHECKSUM	74
#define EVT_ERROR_THROTTLE_LOW_LIMIT		75
#define EVT_ERROR_THROTTLE_HIGH_LIMIT		76
#define EVT_ERROR_WATCHDOG_TRIGGERED		77
#define EVT_ERROR_EXTCOM_CHEKSUM			78
#define EVT_ERROR_EXTCOM_DISCARD			79


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
#define EVT_DATA_MAIN_LOOP_TIME				140
#define EVT_DATA_THROTTLE_ADC				141
#define EVT_DATA_LVC_LIMITING				142
#define EVT_DATA_SHIFT_SENSOR				143
#define EVT_DATA_BBSHD_THERMISTOR			144
#define EVT_DATA_VOLTAGE					145
#define EVT_DATA_CALIBRATE_VOLTAGE			146
#define EVT_DATA_TORQUE_ADC					147
#define EVT_DATA_TORQUE_ADC_CALIBRATED		148


void eventlog_init(bool enabled);

bool eventlog_is_enabled();
void eventlog_set_enabled(bool enabled);

void eventlog_write(uint8_t evt);
void eventlog_write_data(uint8_t evt, int16_t data);


#endif
