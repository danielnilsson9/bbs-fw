/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "stc15.h"

#include "system.h"
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

	system_init();
	watchdog_init();

	pas_init();
	throttle_init();
	motor_init(MAX_CURRENT_X1000, LOW_VOLTAGE_CUTOFF);
	app_init();

	motor_enable();
	motor_set_target_current(0x64);


	while (1)
	{
		motor_process();
		app_process();

		motor_set_target_speed(throttle_read());

		watchdog_yeild();

		system_delay_ms(10);
	}
}