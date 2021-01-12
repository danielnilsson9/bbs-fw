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

#define EVT_ERROR_INIT_MOTOR				64
#define EVT_ERROR_CHANGE_TARGET_SPEED		65
#define EVT_ERROR_CHANGE_TARGET_CURRENT		66

#define EVT_DATA_TARGET_CURRENT				128
#define EVT_DATA_TARGET_SPEED				129
#define EVT_DATA_MOTOR_STATUS				130


void eventlog_init(bool enabled);

void eventlog_set_enabled(bool enabled);

void eventlog_write(uint8_t evt);
void eventlog_write_data(uint8_t evt, int16_t data);


#endif
