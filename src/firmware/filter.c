#include "filter.h"
#include <math.h>

//int16_t filter_exponential_update(int16_t value, __xdata exponential_filter_t* __xdata filter)
//{
//	filter->current = (100 * filter->weight * (int32_t)value + (100 - filter->weight) * filter->current + 50) / 100;
//	return (int16_t)((filter->current + 50) / 100);
//}

//float filter_kalman_update(__xdata float value, __xdata kalman_filter_t* __xdata filter)
//{
//	filter->kalman_gain = filter->err_estimate / (filter->err_estimate + filter->err_measure);
//	filter->current_estimate = filter->last_estimate + filter->kalman_gain * (value - filter->last_estimate);
//	filter->err_estimate = (1.0 - filter->kalman_gain) * filter->err_estimate + fabsf(filter->last_estimate - filter->current_estimate) * filter->q;
//	filter->last_estimate = filter->current_estimate;
//
//	return filter->current_estimate;
//}
