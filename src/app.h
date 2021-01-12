/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */



#ifndef _APP_H_
#define _APP_H_

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
#define ASSIST_PUSH	0x10

#define MODE_ECO		0x01
#define MODE_SPORT		0x02

void app_init();

void app_process();

void app_set_assist_level(uint8_t level);
void app_set_lights(bool on);
void app_set_operation_mode(uint8_t mode);
void app_set_wheel_max_speed_ppm(uint16_t value);


#endif
