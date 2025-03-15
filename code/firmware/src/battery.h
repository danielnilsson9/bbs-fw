/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _BATTERY_H_
#define _BATTERY_H_

#include <stdint.h>

void battery_init();
void battery_process();

uint8_t battery_get_percent();
uint8_t battery_get_mapped_percent();

#endif
