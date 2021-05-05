#ifndef _FILTER_H_
#define _FILTER_H_

#include "stc15.h"

#include <stdint.h>

typedef struct
{
	int8_t weight;
	int32_t current;
} exponential_filter_t;

int16_t filter_exponential_update(int16_t value, __xdata exponential_filter_t* __xdata filter);

//typedef struct
//{
//	float err_measure;
//	float err_estimate;
//	float q;
//	float current_estimate;
//	float last_estimate;
//	float kalman_gain;
//} kalman_filter_t;
//
//float filter_kalman_update(__xdata float value, __xdata kalman_filter_t* __xdata filter);

#endif


