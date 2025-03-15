/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _THROTTLE_H_
#define _THROTTLE_H_

#include <stdbool.h>
#include <stdint.h>

void throttle_init(uint16_t min_mv, uint16_t max_mv);

bool throttle_ok();
uint8_t throttle_read();

uint8_t throttle_map_response(uint8_t throttle_percent);

#endif
