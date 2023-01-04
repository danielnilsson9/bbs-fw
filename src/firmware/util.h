/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

#define MAP16(x, in_min, in_max, out_min, out_max)	((((int16_t)x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))
#define MAP32(x, in_min, in_max, out_min, out_max)	((((int32_t)x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

#define EXPAND_U16(high, low) ((((uint16_t)high) << 8) | (uint8_t)low)
#define EXPAND_I16(high, low) ((int16_t)EXPAND_U16(high,low))

#define ABS(x) (x) < 0 ? -(x) : (x)

#define MAX(x, y) (x) > (y) ? (x) : (y)
#define MIN(x, y) (x) < (y) ? (x) : (y)

// Low pass filter
// value + (new_value - value) / n;
#define EXPONENTIAL_FILTER(value, new_value, n)		(value) + ((new_value) - (value)) / (n)		

#endif

