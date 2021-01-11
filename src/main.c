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
#include "pas.h"
#include "throttle.h"
#include "uart.h"
#include "pins.h"

#define MAX_CURRENT_X1000		30000
#define LOW_VOLTAGE_CUTOFF		24


void main(void)
{
	uart1_open(9600);

	eventlog_init();
	eventlog_set_enabled(false);

	system_init();
	watchdog_init();

	pas_init();
	throttle_init();
	motor_init(MAX_CURRENT_X1000, LOW_VOLTAGE_CUTOFF);
	app_init();

	motor_disable();
	motor_set_target_current(0x64);
	motor_set_target_speed(60);

	while (1)
	{
		motor_process();
		app_process();

		if (pas_is_pedaling_forwards())
		{
			uart1_write(0xff);
			uart1_write(pas_get_rpm());
			motor_enable();
		}
		else if (pas_is_pedaling_backwards())
		{
			uart1_write(0x00);
			uart1_write(pas_get_rpm());
			motor_disable();
		}
		else
		{
			motor_disable();
		}

		uint8_t v = throttle_read();
		//uart1_write(v);

		//motor_set_target_speed(v);

		watchdog_yeild();

		system_delay_ms(10);
	}
}
