/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "cfgstore.h"
#include "eeprom.h"
#include "eventlog.h"
#include "uart.h"

#include <string.h>


#define CONFIG_EEPROM_PAGE		0


typedef struct
{
	uint8_t version;
	uint8_t length;
	uint8_t checksum;
} config_header_t;


static __xdata config_header_t header;
__xdata config_t g_config;


static bool read_config();
static bool write_config();
static void load_default_config();

void cfgstore_init()
{
	if (!read_config())
	{
		cfgstore_reset();
	}
	else
	{
		eventlog_write(EVT_MSG_CONFIG_READ);
	}
}


bool cfgstore_reset()
{
	load_default_config();
	if (write_config())
	{
		eventlog_write(EVT_MSG_CONFIG_RESET);
		return true;
	}

	return false;
}

bool cfgstore_save()
{
	return write_config();
}


static bool read_config()
{
	uint8_t __xdata read_offset = 0;
	__xdata uint8_t* ptr = 0;
	uint8_t i = 0;
	int data;

	if (!eeprom_select_page(CONFIG_EEPROM_PAGE))
	{
		eventlog_write(EVT_ERROR_CONFIG_READ_EEPROM);
		return false;
	}

	ptr = (uint8_t*)&header;
	for (i = 0; i < sizeof(config_header_t); ++i)
	{
		data = eeprom_read_byte(read_offset);
		if (data < 0)
		{
			eventlog_write(EVT_ERROR_CONFIG_READ_EEPROM);
			return false;
		}
		*ptr = (uint8_t)data;
		++read_offset;
		++ptr;
	}

	// verify header ok
	if (header.version != CONFIG_VERSION)
	{
		eventlog_write(EVT_ERROR_CONFIG_VERSION);
		return false;
	}

	if (header.length != sizeof(config_t))
	{
		eventlog_write(EVT_ERROR_CONFIG_VERSION);
		return false;
	}

	uint8_t checksum = 0;

	ptr = (uint8_t*)&g_config;
	for (i = 0; i < sizeof(config_t); ++i)
	{
		data = eeprom_read_byte(read_offset);
		if (data < 0)
		{
			eventlog_write(EVT_ERROR_CONFIG_READ_EEPROM);
			return false;
		}

		checksum += (uint8_t)data;
		*ptr = (uint8_t)data;
		++read_offset;
		++ptr;
	}

	if (header.checksum != checksum)
	{
		eventlog_write(EVT_ERROR_CONFIG_CHECKSUM);
		return false;
	}

	return true;
}

static bool write_config()
{
	uint8_t write_offset = 0;
	__xdata uint8_t* ptr = 0;
	uint8_t i = 0;

	header.version = CONFIG_VERSION;
	header.length = sizeof(config_t);
	header.checksum = 0;

	if (!eeprom_select_page(CONFIG_EEPROM_PAGE))
	{
		eventlog_write(EVT_ERROR_CONFIG_WRITE_EEPROM);
		return false;
	}

	if (!eeprom_erase_page())
	{
		eventlog_write(EVT_ERROR_CONFIG_ERASE_EEPROM);
		return false;
	}

	write_offset += sizeof(config_header_t);

	ptr = (uint8_t*)&g_config;
	for (i = 0; i < sizeof(config_t); ++i)
	{
		if (!eeprom_write_byte(write_offset, *ptr))
		{
			eventlog_write(EVT_ERROR_CONFIG_WRITE_EEPROM);
			return false;
		}

		header.checksum += *ptr;
		++write_offset;
		++ptr;
	}

	write_offset = 0;
	ptr = (uint8_t*)&header;
	for (i = 0; i < sizeof(config_header_t); ++i)
	{
		if (!eeprom_write_byte(write_offset, *ptr))
		{
			eventlog_write(EVT_ERROR_CONFIG_WRITE_EEPROM);
			return false;
		}

		++write_offset;
		++ptr;
	}

	eventlog_write(EVT_MSG_CONFIG_WRITTEN);

	return true;
}

static void load_default_config()
{
	g_config.use_freedom_units = 0;

	g_config.max_current_amps = 25;
	g_config.low_cut_off_V = 42;

	g_config.use_gear_sensor = 1;
	g_config.gear_sensor_delay = 350;

	g_config.use_speed_sensor = 1;
	g_config.use_display = 1;
	g_config.use_push_walk = 1;

	g_config.wheel_size_inch_x10 = 263;
	g_config.speed_sensor_signals = 1;
	g_config.max_speed_kph = 100;

	g_config.pas_start_delay_pulses = 6;
	g_config.pas_stop_delay = 500;

	g_config.throttle_start_voltage_mv = 1100;
	g_config.throttle_end_voltage_mv = 3500;
	g_config.throttle_start_percent = 10;

	//g_config.assist_mode_select = ASSIST_MODE_SELECT_OFF;
	//g_config.assist_mode_select = ASSIST_MODE_SELECT_STANDARD;
	//g_config.assist_mode_select = ASSIST_MODE_SELECT_LIGHTS;
	g_config.assist_mode_select = ASSIST_MODE_SELECT_PAS0_LIGHT;
	g_config.assist_startup_level = 1;

	memset(&g_config.assist_levels, 0, 20 * sizeof(assist_level_t));

	static __xdata uint8_t current_limits_standard[9] = { 15, 20, 30, 40, 50, 60, 70, 85, 100 };
	static __xdata uint8_t cadence_limits_standard[9] = { 60, 60, 60, 60, 60, 70, 80, 90, 100 };
	static __xdata uint8_t speed_limits_standard[9] = { 30, 30, 30, 30, 30, 35, 40, 45, 100 };

	static __xdata uint8_t current_limits_sport[9] = { 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	static __xdata uint8_t cadence_limits_sport[9] = { 100, 100, 100, 100, 100, 100, 100, 100, 100 };
	static __xdata uint8_t speed_limits_sport[9] = { 100, 100, 100, 100, 100, 100, 100, 100, 100 };

	for (uint8_t i = 0; i < 9; ++i)
	{
		g_config.assist_levels[0][i+1].flags = ASSIST_FLAG_PAS;
		g_config.assist_levels[0][i+1].target_current_percent = current_limits_standard[i];
		g_config.assist_levels[0][i+1].max_cadence_percent = cadence_limits_standard[i];
		g_config.assist_levels[0][i+1].max_speed_percent = speed_limits_standard[i];
		g_config.assist_levels[0][i+1].max_throttle_current_percent = current_limits_standard[i];

		g_config.assist_levels[1][i+1].flags = ASSIST_FLAG_PAS | ASSIST_FLAG_THROTTLE;
		g_config.assist_levels[1][i+1].target_current_percent = current_limits_sport[i];
		g_config.assist_levels[1][i+1].max_cadence_percent = cadence_limits_sport[i];
		g_config.assist_levels[1][i+1].max_speed_percent = speed_limits_sport[i];
		g_config.assist_levels[1][i+1].max_throttle_current_percent = current_limits_sport[i];
	}
}
