/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "cfgstore.h"
#include "eeprom.h"
#include "eventlog.h"
#include "uart.h"
#include "fwconfig.h"

#include <string.h>

#define EEPROM_CONFIG_PAGE		0
#define EEPROM_PSTATE_PAGE		1

#define EEPROM_OK					0
#define EEPROM_ERROR_SELECT_PAGE	1
#define EEPROM_ERROR_READ			2
#define EEPROM_ERROR_VERSION		3
#define EEPROM_ERROR_LENGHT			4
#define EEPROM_ERROR_CHECKSUM		5
#define EEPROM_ERROR_ERASE			6
#define EEPROM_ERROR_WRITE			7


static const uint8_t default_current_limits[] = { 7, 10, 14, 19, 26, 36, 50, 70, 98 };

#if HAS_TORQUE_SENSOR
static const uint8_t default_torque_factors[] = { 10, 15, 23, 44, 57, 74, 88, 105, 126 };
#endif

typedef struct
{
	uint8_t version;
	uint8_t length;
	uint8_t checksum;
} header_t;

static header_t header;

config_t g_config;
pstate_t g_pstate;

static uint8_t read(uint8_t page, uint8_t version, uint8_t* dst, uint8_t size);
static uint8_t write(uint8_t page, uint8_t version, uint8_t* src, uint8_t size);

static bool read_config();
static bool write_config();
static void load_default_config();

static bool read_pstate();
static bool write_pstate();
static void load_default_pstate();

void cfgstore_init()
{
	if (!read_config())
	{
		cfgstore_reset_config();
	}

	if (!read_pstate())
	{
		cfgstore_reset_pstate();
	}
}

bool cfgstore_reset_config()
{
	load_default_config();
	if (write_config())
	{
		eventlog_write(EVT_MSG_CONFIG_RESET);
		return true;
	}

	return false;
}

bool cfgstore_save_config()
{
	return write_config();
}

bool cfgstore_reset_pstate()
{
	load_default_pstate();
	return write_pstate();
}

bool cfgstore_save_pstate()
{
	return write_pstate();
}

static bool read_config()
{
	eventlog_write(EVT_MSG_CONFIG_READ_BEGIN);

	uint8_t res = read(EEPROM_CONFIG_PAGE, CONFIG_VERSION, (uint8_t*)&g_config, sizeof(config_t));
	switch (res)
	{
	default:
		eventlog_write(EVT_ERROR_EEPROM_READ);
		break;
	case EEPROM_ERROR_VERSION:
		eventlog_write(EVT_ERROR_EEPROM_VERIFY_VERSION);
		break;
	case EEPROM_ERROR_LENGHT:
	case EEPROM_ERROR_CHECKSUM:
		eventlog_write(EVT_ERROR_EEPROM_VERIFY_CHECKSUM);
		break;
	case EEPROM_OK:
		eventlog_write(EVT_MSG_CONFIG_READ_DONE);
		break;
	}

	return res == EEPROM_OK;
}

static bool write_config()
{
	eventlog_write(EVT_MSG_CONFIG_WRITE_BEGIN);

	uint8_t res = write(EEPROM_CONFIG_PAGE, CONFIG_VERSION, (uint8_t*)&g_config, sizeof(config_t));
	switch (res)
	{
	default:
		eventlog_write(EVT_ERROR_EEPROM_WRITE);
		break;
	case EEPROM_ERROR_ERASE:
		eventlog_write(EVT_ERROR_EEPROM_ERASE);
		break;
	case EEPROM_OK:
		eventlog_write(EVT_MSG_CONFIG_WRITE_DONE);
		break;
	}

	return res == EEPROM_OK;
}

static void load_default_config()
{
	g_config.use_freedom_units = 0;

#if defined(BBSHD)
	g_config.max_current_amps = 30;
#else
	g_config.max_current_amps = 20;
#endif

	g_config.current_ramp_amps_s = 10;
	g_config.max_battery_x100v_u16l = (uint8_t)5460;
	g_config.max_battery_x100v_u16h = (uint8_t)(5460 >> 8);
	g_config.low_cut_off_v = 42;

	g_config.use_display = 1;
	g_config.use_speed_sensor = 1;
	g_config.use_shift_sensor = 1;
	g_config.use_push_walk = 1;
	g_config.use_temperature_sensor = TEMPERATURE_SENSOR_CONTR | TEMPERATURE_SENSOR_MOTOR;

	g_config.wheel_size_inch_x10_u16l = (uint8_t)280;
	g_config.wheel_size_inch_x10_u16h = (uint8_t)(280 >> 8);

	g_config.speed_sensor_signals = 1;
	g_config.max_speed_kph = 100;

	g_config.pas_start_delay_pulses = 5;
	g_config.pas_stop_delay_x100s = 20;
	g_config.pas_keep_current_percent = 60;
	g_config.pas_keep_current_cadence_rpm = 40;

	g_config.throttle_start_voltage_mv_u16l = (uint8_t)900;
	g_config.throttle_start_voltage_mv_u16h = (uint8_t)(900 >> 8);

	g_config.throttle_end_voltage_mv_u16l = (uint8_t)3600;
	g_config.throttle_end_voltage_mv_u16h = (uint8_t)(3600 >> 8);
	g_config.throttle_start_percent = 1;

	g_config.shift_interrupt_duration_ms = 600;
	g_config.shift_interrupt_current_threshold_percent = 10;

	g_config.show_temperature_push_walk = 0;

	g_config.assist_mode_select = ASSIST_MODE_SELECT_OFF;
	g_config.assist_startup_level = 3;

	memset(&g_config.assist_levels, 0, 20 * sizeof(assist_level_t));

	for (uint8_t i = 0; i < 9; ++i)
	{
		g_config.assist_levels[0][i+1].flags = ASSIST_FLAG_PAS | ASSIST_FLAG_THROTTLE;
		g_config.assist_levels[0][i+1].max_cadence_percent = 100;
		g_config.assist_levels[0][i+1].max_speed_percent = 100;
		g_config.assist_levels[0][i+1].max_throttle_current_percent = 100;

#if HAS_TORQUE_SENSOR
		g_config.assist_levels[0][i+1].flags |= ASSIST_FLAG_PAS_TORQUE;
		g_config.assist_levels[0][i+1].target_current_percent = 100;
		g_config.assist_levels[0][i+1].torque_amplification_factor_x10 = default_torque_factors[i];
#else
		g_config.assist_levels[0][i+1].target_current_percent = default_current_limits[i];
		g_config.assist_levels[0][i+1].torque_amplification_factor_x10 = 0;
#endif	
	}
}


static bool read_pstate()
{
	eventlog_write(EVT_MSG_PSTATE_READ_BEGIN);

	uint8_t res = read(EEPROM_PSTATE_PAGE, PSTATE_VERSION, (uint8_t*)&g_pstate, sizeof(pstate_t));
	switch (res)
	{
	default:
		eventlog_write(EVT_ERROR_EEPROM_READ);
		break;
	case EEPROM_ERROR_VERSION:
		eventlog_write(EVT_ERROR_EEPROM_VERIFY_VERSION);
		break;
	case EEPROM_ERROR_LENGHT:
	case EEPROM_ERROR_CHECKSUM:
		eventlog_write(EVT_ERROR_EEPROM_VERIFY_CHECKSUM);
		break;
	case EEPROM_OK:
		eventlog_write(EVT_MSG_PSTATE_READ_DONE);
		break;
	}

	return res == EEPROM_OK;
}

static bool write_pstate()
{
	eventlog_write(EVT_MSG_PSTATE_WRITE_BEGIN);

	uint8_t res = write(EEPROM_PSTATE_PAGE, PSTATE_VERSION, (uint8_t*)&g_pstate, sizeof(pstate_t));
	switch (res)
	{
	default:
		eventlog_write(EVT_ERROR_EEPROM_WRITE);
		break;
	case EEPROM_ERROR_ERASE:
		eventlog_write(EVT_ERROR_EEPROM_ERASE);
		break;
	case EEPROM_OK:
		eventlog_write(EVT_MSG_PSTATE_WRITE_DONE);
		break;
	}

	return res == EEPROM_OK;

}

static void load_default_pstate()
{
	g_pstate.adc_voltage_calibration_steps_x100_i16l = 0;
	g_pstate.adc_voltage_calibration_steps_x100_i16h = 0;
}



static uint8_t read(uint8_t page, uint8_t version, uint8_t* dst, uint8_t size)
{
	uint8_t read_offset = 0;
	uint8_t* ptr = 0;
	uint8_t i = 0;
	int data;

	if (!eeprom_select_page(page))
	{
		return EEPROM_ERROR_SELECT_PAGE;
	}

	ptr = (uint8_t*)&header;
	for (i = 0; i < sizeof(header_t); ++i)
	{
		data = eeprom_read_byte(read_offset);
		if (data < 0)
		{
			return EEPROM_ERROR_READ;
		}
		*ptr = (uint8_t)data;
		++read_offset;
		++ptr;
	}

	// verify header ok
	if (header.version != version)
	{
		return EEPROM_ERROR_VERSION;
	}

	if (header.length != size)
	{
		return EEPROM_ERROR_LENGHT;
	}

	uint8_t checksum = 0;

	ptr = dst;
	for (i = 0; i < size; ++i)
	{
		data = eeprom_read_byte(read_offset);
		if (data < 0)
		{
			return EEPROM_ERROR_READ;
		}

		checksum += (uint8_t)data;
		*ptr = (uint8_t)data;
		++read_offset;
		++ptr;
	}

	if (header.checksum != checksum)
	{
		return EEPROM_ERROR_CHECKSUM;
	}

	return EEPROM_OK;
}

static uint8_t write(uint8_t page, uint8_t version, uint8_t* src, uint8_t size)
{
	uint8_t write_offset = 0;
	uint8_t* ptr = 0;
	uint8_t i = 0;

	header.version = version;
	header.length = size;
	header.checksum = 0;

	if (!eeprom_select_page(page))
	{
		return EEPROM_ERROR_SELECT_PAGE;
	}

	if (!eeprom_erase_page())
	{
		return EEPROM_ERROR_ERASE;
	}

	write_offset += sizeof(header_t);

	ptr = src;
	for (i = 0; i < size; ++i)
	{
		if (!eeprom_write_byte(write_offset, *ptr))
		{
			eeprom_end_write();
			return EEPROM_ERROR_WRITE;
		}

		header.checksum += *ptr;
		++write_offset;
		++ptr;
	}

	write_offset = 0;
	ptr = (uint8_t*)&header;
	for (i = 0; i < sizeof(header_t); ++i)
	{
		if (!eeprom_write_byte(write_offset, *ptr))
		{
			eeprom_end_write();
			return EEPROM_ERROR_WRITE;
		}

		++write_offset;
		++ptr;
	}

	eeprom_end_write();

	return EEPROM_OK;
}
