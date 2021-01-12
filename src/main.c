/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "stc15.h"

#include "system.h"
#include "eventlog.h"
#include "app.h"
#include "watchdog.h"
#include "motor.h"
#include "extcom.h"
#include "sensors.h"
#include "throttle.h"
#include "uart.h"
#include "pins.h"

#define MAX_CURRENT_X1000		30000
#define LOW_VOLTAGE_CUTOFF		24


void main(void)
{


	system_init();
	watchdog_init();

	extcom_init();
	eventlog_init(false);

	sensors_init();
	throttle_init();
	motor_init(MAX_CURRENT_X1000, LOW_VOLTAGE_CUTOFF);
	app_init();

	motor_enable();
	motor_set_target_current(0x64);
	motor_set_target_speed(0);

	while (1)
	{
		motor_process();
		extcom_process();
		app_process();

		/*if (pas_is_pedaling_forwards())
		{
			uart1_write(0xff);
			uart1_write(pas_get_cadence_rpm());
			motor_enable();
		}
		else if (pas_is_pedaling_backwards())
		{
			uart1_write(0x00);
			uart1_write(pas_get_cadence_rpm());
			motor_disable();
		}
		else
		{
			motor_disable();
		}*/

		uart1_write(speed_sensor_get_ticks_per_minute());


		uint8_t v = throttle_read();
		//uart1_write(v);

		motor_set_target_speed(v);

		watchdog_yeild();

		system_delay_ms(10);
	}
}
