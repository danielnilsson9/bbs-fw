/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "extcom.h"
#include "cfgstore.h"
#include "eventlog.h"
#include "uart.h"
#include "system.h"
#include "sensors.h"
#include "motor.h"
#include "battery.h"
#include "app.h"
#include "util.h"
#include "version.h"
#include "intellisense.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define KEEP		0
#define DISCARD		-1


#define BUFFER_SIZE			192
#define DISCARD_TIMEOUT_MS	50

#define REQUEST_TYPE_READ						0x01
#define REQUEST_TYPE_WRITE						0x02

#define REQUEST_TYPE_BAFANG_READ				0x11
#define REQUEST_TYPE_BAFANG_WRITE				0x16


// Firmware config tool communication
#define OPCODE_READ_FW_VERSION					0x01
#define OPCODE_READ_EVTLOG_ENABLE				0x02
#define OPCODE_READ_CONFIG						0x03
#define OPCODE_READ_STATUS						0x04

#define OPCODE_WRITE_EVTLOG_ENABLE				0xf0
#define OPCODE_WRITE_CONFIG						0xf1
#define OPCODE_WRITE_RESET_CONFIG				0xf2
#define OPCODE_WRITE_ADC_VOLTAGE_CALIBRATION	0xf3


// Bafang display communication
#define OPCODE_BAFANG_DISPLAY_READ_STATUS		0x08
#define OPCODE_BAFANG_DISPLAY_READ_CURRENT		0x0a
#define OPCODE_BAFANG_DISPLAY_READ_BATTERY		0x11
#define OPCODE_BAFANG_DISPLAY_READ_SPEED		0x20
#define OPCODE_BAFANG_DISPLAY_READ_UNKNOWN1		0x21
#define OPCODE_BAFANG_DISPLAY_READ_RANGE		0x22
#define OPCODE_BAFANG_DISPLAY_READ_CALORIES		0x24
#define OPCODE_BAFANG_DISPLAY_READ_UNKNOWN3		0x25
#define OPCODE_BAFANG_DISPLAY_READ_MOVING		0x31

#define OPCODE_BAFANG_DISPLAY_WRITE_PAS			0x0b
#define OPCODE_BAFANG_DISPLAY_WRITE_MODE		0x0c
#define OPCODE_BAFANG_DISPLAY_WRITE_LIGHTS		0x1a
#define OPCODE_BAFANG_DISPLAY_WRITE_SPEED_LIM	0x1f

// Bafang config tool communication (not supported, just discard messages)
#define OPCODE_BAFANG_TOOL_READ_CONNECT			0x51
#define OPCODE_BAFANG_TOOL_READ_BASIC			0x52
#define OPCODE_BAFANG_TOOL_READ_PAS				0x53
#define OPCODE_BAFANG_TOOL_READ_THROTTLE		0x54

#define OPCODE_BAFANG_TOOL_WRITE_BASIC			0x52
#define OPCODE_BAFANG_TOOL_WRITE_PAS			0x53
#define OPCODE_BAFANG_TOOL_WRITE_THROTTLE		0x54



static uint8_t msg_len;
static uint8_t msgbuf[BUFFER_SIZE];
static uint32_t last_recv_ms;
static uint32_t discard_until_ms;

static uint8_t compute_checksum(uint8_t* buf, uint8_t length);
static void write_uart_and_increment_checksum(uint8_t data, uint8_t* checksum);

static int16_t try_process_request();
static int16_t try_process_read_request();
static int16_t try_process_write_request();
static int16_t try_process_bafang_read_request();
static int16_t try_process_bafang_write_request();


static int16_t process_read_fw_version();
static int16_t process_read_evtlog_enable();
static int16_t process_read_config();
static int16_t process_read_status();

static int16_t process_write_evtlog_enable();
static int16_t process_write_config();
static int16_t process_write_reset_config();
static int16_t process_write_adc_voltage_calibration();


static int16_t process_bafang_display_read_status();
static int16_t process_bafang_display_read_current();
static int16_t process_bafang_display_read_battery();
static int16_t process_bafang_display_read_speed();
static int16_t process_bafang_display_read_unknown1();
static int16_t process_bafang_display_read_range();
static int16_t process_bafang_display_read_calories();
static int16_t process_bafang_display_read_unknown3();
static int16_t process_bafang_display_read_moving();

static int16_t process_bafang_display_write_pas();
static int16_t process_bafang_display_write_mode();
static int16_t process_bafang_display_write_lights();
static int16_t process_bafang_display_write_speed_limit();

void extcom_init()
{
	msg_len = 0;
	last_recv_ms = 0;
	discard_until_ms = 0;

	// Bafang standard baud rate
	uart_open(1200);


	// Wait one second for config tool connection.
	// This is here to that the config tool can enable
	// the event log before system proceeds with initialization.
	uint32_t end = system_ms() + 1000;
	while (system_ms() < end)
	{
		extcom_process();
		system_delay_ms(10);
	}
}

void extcom_process()
{
	uint32_t now = system_ms();

	while (uart_available())
	{
		if (msg_len == BUFFER_SIZE || (discard_until_ms != 0 && now < discard_until_ms))
		{
			// communication error, reset
			msg_len = 0;
			while (uart_available()) uart_read();
		}
		else
		{
			msgbuf[msg_len++] = uart_read();
			last_recv_ms = now;
			discard_until_ms = 0;
		}	
	}

	if (msg_len > 0 && now - last_recv_ms > 100)
	{
		// communication error, reset
		msg_len = 0;
	}

	int16_t res = try_process_request();
	if (res == DISCARD)
	{
		msg_len = 0;
		last_recv_ms = 0;
		// Discard received data for the next DISCARD_TIMEOUT_MS milliseconds
		discard_until_ms = now + DISCARD_TIMEOUT_MS;

		eventlog_write(EVT_ERROR_EXTCOM_DISCARD);
	}
	else if (res > 0)
	{
		if (res < msg_len)
		{
			// will not occur due to request/response communication
			memcpy(msgbuf, msgbuf + res, msg_len - res);
			msg_len -= res;
		}
		else
		{
			msg_len = 0;
			last_recv_ms = 0;
		}
	}
}


static uint8_t compute_checksum(uint8_t* buf, uint8_t length)
{
	uint8_t result = 0;

	for (uint8_t i = 0; i < length; ++i)
	{
		result += buf[i];
	}

	return result;
}

static void write_uart_and_increment_checksum(uint8_t data, uint8_t* checksum)
{
	*checksum += data;
	uart_write(data);
}

static int16_t try_process_request()
{
	if (msg_len < 1)
	{
		return KEEP;
	}

	switch (msgbuf[0])
	{
	case REQUEST_TYPE_READ:
		return try_process_read_request();
	case REQUEST_TYPE_WRITE:
		return try_process_write_request();
	case REQUEST_TYPE_BAFANG_READ:
		return try_process_bafang_read_request();
	case REQUEST_TYPE_BAFANG_WRITE:
		return try_process_bafang_write_request();
	}

	return DISCARD; // unknown message
}

static int16_t try_process_read_request()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	switch (msgbuf[1])
	{
	case OPCODE_READ_FW_VERSION:
		return process_read_fw_version();
	case OPCODE_READ_EVTLOG_ENABLE:
		return process_read_evtlog_enable();
	case OPCODE_READ_CONFIG:
		return process_read_config();
	case OPCODE_READ_STATUS:
		return process_read_status();
	}

	return DISCARD;
}

static int16_t try_process_write_request()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	switch (msgbuf[1])
	{
	case OPCODE_WRITE_EVTLOG_ENABLE:
		return process_write_evtlog_enable();
	case OPCODE_WRITE_CONFIG:
		return process_write_config();
	case OPCODE_WRITE_RESET_CONFIG:
		return process_write_reset_config();
	case OPCODE_WRITE_ADC_VOLTAGE_CALIBRATION:
		return process_write_adc_voltage_calibration();
	}

	return DISCARD;
}

static int16_t try_process_bafang_read_request()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	switch (msgbuf[1])
	{
	case OPCODE_BAFANG_DISPLAY_READ_STATUS:
		return process_bafang_display_read_status();
	case OPCODE_BAFANG_DISPLAY_READ_CURRENT:
		return process_bafang_display_read_current();
	case OPCODE_BAFANG_DISPLAY_READ_BATTERY:
		return process_bafang_display_read_battery();
	case OPCODE_BAFANG_DISPLAY_READ_SPEED:
		return process_bafang_display_read_speed();
	case OPCODE_BAFANG_DISPLAY_READ_UNKNOWN1:
		return process_bafang_display_read_unknown1();
	case OPCODE_BAFANG_DISPLAY_READ_RANGE:
		return process_bafang_display_read_range();
	case OPCODE_BAFANG_DISPLAY_READ_CALORIES:
		return process_bafang_display_read_calories();
	case OPCODE_BAFANG_DISPLAY_READ_UNKNOWN3:
		return process_bafang_display_read_unknown3();
	case OPCODE_BAFANG_DISPLAY_READ_MOVING:
		return process_bafang_display_read_moving();
	}

	return DISCARD;
}

static int16_t try_process_bafang_write_request()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	switch (msgbuf[1])
	{
	case OPCODE_BAFANG_DISPLAY_WRITE_PAS:
		return process_bafang_display_write_pas();
	case OPCODE_BAFANG_DISPLAY_WRITE_MODE:
		return process_bafang_display_write_mode();
	case OPCODE_BAFANG_DISPLAY_WRITE_LIGHTS:
		return process_bafang_display_write_lights();
	case OPCODE_BAFANG_DISPLAY_WRITE_SPEED_LIM:
		return process_bafang_display_write_speed_limit();
	}

	return DISCARD;
}



static int16_t process_read_fw_version()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart_and_increment_checksum(OPCODE_READ_FW_VERSION, &checksum);
		write_uart_and_increment_checksum(VERSION_MAJOR, &checksum);
		write_uart_and_increment_checksum(VERSION_MINOR, &checksum);
		write_uart_and_increment_checksum(VERSION_PATCH, &checksum);
		write_uart_and_increment_checksum(CONFIG_VERSION, &checksum);
		write_uart_and_increment_checksum(CTRL_TYPE, &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 3;
}

static int16_t process_read_evtlog_enable()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart_and_increment_checksum(OPCODE_READ_EVTLOG_ENABLE, &checksum);
		write_uart_and_increment_checksum((uint8_t)eventlog_is_enabled(), &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 3;
}

static int16_t process_read_config()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart_and_increment_checksum(OPCODE_READ_CONFIG, &checksum);
		write_uart_and_increment_checksum(CONFIG_VERSION, &checksum);
		write_uart_and_increment_checksum(sizeof(config_t), &checksum);

		uint8_t* cfg = (uint8_t*)&g_config;
		for (uint8_t i = 0; i < sizeof(config_t); ++i)
		{
			write_uart_and_increment_checksum(*(cfg + i), &checksum);
		}

		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 3;
}

static int16_t process_read_status()
{
	// :TODO:
	return 0;
}

static int16_t process_write_evtlog_enable()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 3) == msgbuf[3])
	{
		eventlog_set_enabled((bool)msgbuf[2]);

		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart_and_increment_checksum(OPCODE_WRITE_EVTLOG_ENABLE, &checksum);
		write_uart_and_increment_checksum(msgbuf[2], &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 4;
}

static int16_t process_write_config()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	uint8_t version = msgbuf[2];
	uint8_t length = msgbuf[3];

	if (msg_len < 4 + length + 1)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, (uint8_t)(4 + sizeof(config_t))) == msgbuf[4 + sizeof(config_t)])
	{
		bool result = false;
		if (version == CONFIG_VERSION && length == sizeof(config_t))
		{
			memcpy(&g_config, msgbuf + 4, sizeof(config_t));
			result = cfgstore_save_config();
		}

		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart_and_increment_checksum(OPCODE_WRITE_CONFIG, &checksum);
		write_uart_and_increment_checksum(result, &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 4 + length + 1;
}

static int16_t process_write_reset_config()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{

		bool res = cfgstore_reset_config();

		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart_and_increment_checksum(OPCODE_WRITE_RESET_CONFIG, &checksum);
		write_uart_and_increment_checksum((uint8_t)res, &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 3;
}

static int16_t process_write_adc_voltage_calibration()
{
	if (msg_len < 5)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 4) == msgbuf[4])
	{
		uint16_t actual_volt_x100 = ((uint16_t)msgbuf[2] << 8) | msgbuf[3];

		int16_t calibration_offset = motor_calibrate_battery_voltage(actual_volt_x100);
		g_pstate.adc_voltage_calibration_steps_x100_i16l = (uint8_t)(calibration_offset);
		g_pstate.adc_voltage_calibration_steps_x100_i16h = (uint8_t)(calibration_offset >> 8);

		cfgstore_save_pstate();

		uint8_t checksum = 0;
		write_uart_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart_and_increment_checksum(OPCODE_WRITE_ADC_VOLTAGE_CALIBRATION, &checksum);
		write_uart_and_increment_checksum(msgbuf[2], &checksum);
		write_uart_and_increment_checksum(msgbuf[3], &checksum);
		uart_write(checksum);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 5;
}


static int16_t process_bafang_display_read_status()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uart_write(app_get_status_code());

	return 2;
}

static int16_t process_bafang_display_read_current()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t amp_x2 = (uint8_t)((motor_get_battery_current_x10() * 2) / 10);

	uart_write(amp_x2);
	uart_write(amp_x2); // checksum

	return 2;
}

static int16_t process_bafang_display_read_battery()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t value = battery_get_mapped_percent();

	uart_write(value);
	uart_write(value); // checksum

	return 2;
}

static int16_t process_bafang_display_read_speed()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint16_t speed = 0;

	if (g_config.walk_mode_data_display != WALK_MODE_DATA_SPEED && app_get_assist_level() == ASSIST_PUSH)
	{
		uint16_t data = 0;

		switch (g_config.walk_mode_data_display)
		{
		case WALK_MODE_DATA_TEMPERATURE:
			// Keep temperature in C, farenheit would be out of range
			data = app_get_temperature();
			break;
		case WALK_MODE_DATA_REQUESTED_POWER:
			data = motor_get_target_current();
			break;
		case WALK_MODE_DATA_BATTERY_PERCENT:
			data = battery_get_percent();
			break;
		}

		if (g_config.use_freedom_units)
		{
			// Compensate for kph -> mph conversion display will do.
			data = (data * 161) / 100;
		}

		// T_kph -> rpm
		speed = (uint16_t)(25000.f / (3 * 3.14159f * 1.27f * EXPAND_U16(g_config.wheel_size_inch_x10_u16h, g_config.wheel_size_inch_x10_u16l)) * data);
	}
	else
	{
		speed = speed_sensor_get_rpm_x10() / 10;
	}


	uint8_t checksum = 0;

	write_uart_and_increment_checksum(speed >> 8, &checksum);
	write_uart_and_increment_checksum((uint8_t)speed, &checksum);
	uart_write(checksum + (uint8_t)0x20); // weird checksum

	return 2;
}

static int16_t process_bafang_display_read_unknown1()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uart_write(0x00);
	uart_write(0x00);
	uart_write(0x00); // checksum

	return 3;
}

static int16_t process_bafang_display_read_range()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uint16_t temp = app_get_temperature();
	if (g_config.use_freedom_units)
	{
		// Convert to farenheit and compensate for the km -> miles conversion the diplay will do
		// F_miles = (C * 9/5 + 32) * 161 / 100
		// Approximistation:
		// F_miles = 2.9C + 50.5

		temp = ((290u * temp) + 5050u) / 100u;
	}

	uint8_t checksum = 0;

	write_uart_and_increment_checksum((uint8_t)(temp >> 8), &checksum);
	write_uart_and_increment_checksum((uint8_t)temp, &checksum);
	uart_write(checksum); // checksum

	return 3;
}

static int16_t process_bafang_display_read_calories()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uint8_t checksum = 0;

	// send battery voltage x10 to show in calories field
	uint16_t volt = motor_get_battery_voltage_x10();

	write_uart_and_increment_checksum(volt >> 8, & checksum);
	write_uart_and_increment_checksum(volt & 0xff, & checksum);
	uart_write(checksum); // checksum

	return 3;
}

static int16_t process_bafang_display_read_unknown3()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uart_write(0x00);
	uart_write(0x00);
	uart_write(0x00);
	uart_write(0x00);
	uart_write(0x00); // checksum

	return 3;
}

static int16_t process_bafang_display_read_moving()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t data = speed_sensor_is_moving() ? 0x30 : 0x31;
	uart_write(data);
	uart_write(data); // checksum

	return 2;
}


static int16_t process_bafang_display_write_pas()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 3) == msgbuf[3])
	{
		switch (msgbuf[2])
		{
		case 0x00:
			app_set_assist_level(ASSIST_0);
			break;
		case 0x01:
			app_set_assist_level(ASSIST_1);
			break;
		case 0x0b:
			app_set_assist_level(ASSIST_2);
			break;
		case 0x0c:
			app_set_assist_level(ASSIST_3);
			break;
		case 0x0d:
			app_set_assist_level(ASSIST_4);
			break;
		case 0x02:
			app_set_assist_level(ASSIST_5);
			break;
		case 0x15:
			app_set_assist_level(ASSIST_6);
			break;
		case 0x16:
			app_set_assist_level(ASSIST_7);
			break;
		case 0x17:
			app_set_assist_level(ASSIST_8);
			break;
		case 0x03:
			app_set_assist_level(ASSIST_9);
			break;
		case 0x06:
			app_set_assist_level(ASSIST_PUSH);
			break;
		default:
			// Unsupported level, ignore
			break;
		}
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 4;
}

static int16_t process_bafang_display_write_mode()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 3) == msgbuf[3])
	{
		switch (msgbuf[2])
		{
		case 0x02:
			app_set_operation_mode(OPERATION_MODE_DEFAULT);
			break;
		case 0x04:
			app_set_operation_mode(OPERATION_MODE_SPORT);
			break;
		default:
			// Unsupported mode, ignore
			break;
		}
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}

	return 4;
}

static int16_t process_bafang_display_write_lights()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	// No checksum

	switch (msgbuf[2])
	{
	case 0xf0:
		app_set_lights(false);
		break;
	case 0xf1:
		app_set_lights(true);
		break;
	default:
		return DISCARD; // unsupported state, assume communication error
	}

	return 3;
}

static int16_t process_bafang_display_write_speed_limit()
{
	if (msg_len < 5)
	{
		return KEEP;
	}

	/*
	if (compute_checksum(msgbuf, 4) == msgbuf[4])
	{
		 // Ignoring speed limit requested by display,
		 // Global speed limit is configured in firmware config tool.
		 
		 uint16_t value = ((msgbuf[2] << 8) | msgbuf[3]);
		 app_set_wheel_max_speed_rpm(value);
	}
	else
	{
		eventlog_write(EVT_ERROR_EXTCOM_CHEKSUM);
		return DISCARD;
	}
	*/

	return 5;
}
