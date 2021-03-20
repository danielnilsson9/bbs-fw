/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

#define MAP(x, in_min, in_max, out_min, out_max)	((((int32_t)x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

#endif

