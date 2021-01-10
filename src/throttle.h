/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _THROTTLE_H_
#define _THROTTLE_H_

#include <stdint.h>


void throttle_init();

uint8_t throttle_read();


#endif

