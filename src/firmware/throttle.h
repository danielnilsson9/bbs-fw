/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _THROTTLE_H_
#define _THROTTLE_H_

#include "intellisense.h"
#include <stdint.h>
#include <stdbool.h>

void throttle_init(uint16_t min_mv, uint16_t max_mv);

bool throttle_ok();
uint8_t throttle_read();

#endif

