/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "interrupt.h" // IMPORTANT: interrupt vector declarations must be included from main.c!
#include "timers.h"
#include "system.h"
#include "eeprom.h"
#include "cfgstore.h"
#include "eventlog.h"
#include "app.h"
#include "battery.h"
#include "watchdog.h"
#include "adc.h"
#include "motor.h"
#include "extcom.h"
#include "sensors.h"
#include "throttle.h"
#include "lights.h"
#include "uart.h"
#include "util.h"

#define APP_PROCESS_INTERVAL_MS		5

void main(void)
{
	motor_pre_init();

	watchdog_init();
	timers_init();
	system_init();

	eventlog_init(false);
	extcom_init();

	if (watchdog_triggered())
	{
		// force write watchdog reset to eventlog
		bool prev = eventlog_is_enabled();
		eventlog_set_enabled(true);
		eventlog_write(EVT_ERROR_WATCHDOG_TRIGGERED);
		eventlog_set_enabled(prev);
	}

	eeprom_init();
	cfgstore_init();

	adc_init();
	sensors_init();

	speed_sensor_set_signals_per_rpm(g_config.speed_sensor_signals);
	pas_set_stop_delay((uint16_t)g_config.pas_stop_delay_x100s * 10);

	battery_init();
	throttle_init(
		EXPAND_U16(g_config.throttle_start_voltage_mv_u16h, g_config.throttle_start_voltage_mv_u16l),
		EXPAND_U16(g_config.throttle_end_voltage_mv_u16h, g_config.throttle_end_voltage_mv_u16l)
	);

	motor_init(g_config.max_current_amps * 1000, g_config.low_cut_off_v,
		EXPAND_I16(g_pstate.adc_voltage_calibration_steps_x100_i16h, g_pstate.adc_voltage_calibration_steps_x100_i16l));

	lights_init();

	app_init();

	uint32_t next_app_proccess = system_ms();
	while (1)
	{
		uint32_t now = system_ms();
	
		adc_process();
		motor_process();

		if (now >= next_app_proccess)
		{
			next_app_proccess = now + APP_PROCESS_INTERVAL_MS;

			battery_process();
			sensors_process();
			extcom_process();
			app_process();
		}

		watchdog_yeild();
	}
}
