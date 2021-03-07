/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "eeprom.h"
#include "cfgstore.h"
#include "system.h"
#include "eventlog.h"
#include "app.h"
#include "watchdog.h"
#include "motor.h"
#include "extcom.h"
#include "sensors.h"
#include "throttle.h"
#include "pins.h"
#include "uart.h"


void main(void)
{
	watchdog_init();
	system_init();

	eventlog_init(false);
	extcom_init();

	cfgstore_init();
	config_t* cfg = cfgstore_get();

	sensors_init();
	throttle_init(cfg->throttle_start_voltage_mv, cfg->throttle_end_voltage_mv);
	motor_init(cfg->max_current_amps * 1000, cfg->low_cut_off_V);

	app_init();

	while (1)
	{
		motor_process();
		extcom_process();
		app_process();

		system_delay_ms(10);

		watchdog_yeild();
	}
}
