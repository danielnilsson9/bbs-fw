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
#include "adc.h"
#include "motor.h"
#include "extcom.h"
#include "sensors.h"
#include "throttle.h"
#include "lights.h"
#include "pins.h"
#include "uart.h"


//#define DEBUG_LOOP_TIME_PIN
//#define DEBUG_LOOP_TIME_EVENTLOG


void main(void)
{
	watchdog_init();
	system_init();

	eventlog_init(false);
	extcom_init();

	cfgstore_init();

	adc_init();
	sensors_init();
	speed_sensor_set_signals_per_rpm(g_config.speed_sensor_signals);
	pas_set_stop_delay(g_config.pas_stop_delay_x10ms * 10);

	throttle_init(g_config.throttle_start_voltage_mv, g_config.throttle_end_voltage_mv);
	motor_init(g_config.max_current_amps * 1000, g_config.low_cut_off_V);
	lights_init();

	app_init();


#ifdef DEBUG_LOOP_TIME_PIN
	SET_PIN_OUTPUT(PIN_GEAR_SENSOR);
#endif

#ifdef DEBUG_LOOP_TIME_EVENTLOG
	__xdata uint16_t prev_loop_ms = (uint16_t)system_ms();
	uint8_t loop_counter = 0;
#endif


	while (1)
	{
#ifdef DEBUG_LOOP_TIME_PIN
		SET_PIN_HIGH(PIN_GEAR_SENSOR);
#endif
		adc_process();
		motor_process();
		extcom_process();
		app_process();

#ifdef DEBUG_LOOP_TIME_PIN
		SET_PIN_LOW(PIN_GEAR_SENSOR);
#endif

#ifdef DEBUG_LOOP_TIME_EVENTLOG
		if (++loop_counter == 0)
		{
			eventlog_write_data(EVT_DATA_MAIN_LOOP_TIME, (uint16_t)system_ms() - prev_loop_ms);	
		}
		prev_loop_ms = (uint16_t)system_ms();
#endif

		system_delay_ms(4);
		watchdog_yeild();
	}
}
