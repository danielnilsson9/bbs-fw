/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "extcom.h"
#include "cfgstore.h"
#include "eventlog.h"
#include "stc15.h"
#include "uart.h"
#include "system.h"
#include "sensors.h"
#include "motor.h"
#include "app.h"
#include "util.h"
#include "version.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define KEEP		0
#define DISCARD		-1


#define BUFFER_SIZE 128

#define REQUEST_TYPE_READ						0x01
#define REQUEST_TYPE_WRITE						0x02

#define REQUEST_TYPE_BAFANG_READ				0x11
#define REQUEST_TYPE_BAFANG_WRITE				0x16


// Firmware config tool communication
#define OPCODE_READ_FW_VERSION					0x01
#define OPCODE_READ_EVTLOG_ENABLE				0x02
#define OPCODE_READ_CONFIG						0x03

#define OPCODE_WRITE_EVTLOG_ENABLE				0xf0
#define OPCODE_WRITE_CONFIG						0xf1
#define OPCODE_WRITE_RESET_CONFIG				0xf2


// Bafang display communication
#define OPCODE_BAFANG_DISPLAY_READ_STATUS		0x08
#define OPCODE_BAFANG_DISPLAY_READ_CURRENT		0x0a
#define OPCODE_BAFANG_DISPLAY_READ_BATTERY		0x11
#define OPCODE_BAFANG_DISPLAY_READ_SPEED		0x20
#define OPCODE_BAFANG_DISPLAY_READ_UNKNOWN1		0x21
#define OPCODE_BAFANG_DISPLAY_READ_RANGE		0x22
#define OPCODE_BAFANG_DISPLAY_READ_UNKNOWN2		0x24
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



static __xdata uint8_t msg_len;
static __xdata uint8_t msgbuf[128];
static __xdata uint32_t last_recv;


static uint8_t compute_checksum(uint8_t* buf, uint8_t length);
static void write_uart1_and_increment_checksum(uint8_t data, uint8_t* checksum);

static int8_t try_process_request();
static int8_t try_process_read_request();
static int8_t try_process_write_request();
static int8_t try_process_bafang_read_request();
static int8_t try_process_bafang_write_request();


static int8_t process_read_fw_version();
static int8_t process_read_evtlog_enable();
static int8_t process_read_config();

static int8_t process_write_evtlog_enable();
static int8_t process_write_config();
static int8_t process_write_reset_config();


static int8_t process_bafang_display_read_status();
static int8_t process_bafang_display_read_current();
static int8_t process_bafang_display_read_battery();
static int8_t process_bafang_display_read_speed();
static int8_t process_bafang_display_read_unknown1();
static int8_t process_bafang_display_read_range();
static int8_t process_bafang_display_read_unknown2();
static int8_t process_bafang_display_read_unknown3();
static int8_t process_bafang_display_read_moving();

static int8_t process_bafang_display_write_pas();
static int8_t process_bafang_display_write_mode();
static int8_t process_bafang_display_write_lights();
static int8_t process_bafang_display_write_speed_limit();

void extcom_init()
{
	msg_len = 0;
	last_recv = 0;

	// bafang standard baudrate
	uart1_open(1200);


	// Wait one second for config tool connection.
	// This is here to that the config tool can enable
	// the eventlog before system proceeds with initialization.
	__xdata uint32_t end = system_ms() + 1000;
	while (system_ms() < end)
	{
		extcom_process();
		system_delay_ms(10);
	}
}

void extcom_process()
{
	__xdata uint32_t now = system_ms();

	while (uart1_available())
	{
		if (msg_len == BUFFER_SIZE)
		{
			// communication error, reset
			msg_len = 0;
			while (uart1_available()) uart1_read();
		}
		else
		{
			msgbuf[msg_len++] = uart1_read();
			last_recv = now;
		}	
	}

	if (msg_len > 0 && now - last_recv > 100)
	{
		// communication error, reset
		msg_len = 0;
	}

	int8_t res = try_process_request();
	if (res == DISCARD)
	{
		msg_len = 0;
		last_recv = 0;
	}
	else if (res > 0)
	{
		if ((uint8_t)res < msg_len)
		{
			// will not happend due to request/response communication
			memcpy(msgbuf, msgbuf + res, msg_len - res);
			msg_len -= res;
		}
		else
		{
			msg_len = 0;
			last_recv = 0;
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

static void write_uart1_and_increment_checksum(uint8_t data, uint8_t* checksum)
{
	*checksum += data;
	uart1_write(data);
}

static int8_t try_process_request()
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

static int8_t try_process_read_request()
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
	}

	return DISCARD;
}

static int8_t try_process_write_request()
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
	}

	return DISCARD;
}

static int8_t try_process_bafang_read_request()
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
	case OPCODE_BAFANG_DISPLAY_READ_UNKNOWN2:
		return process_bafang_display_read_unknown2();
	case OPCODE_BAFANG_DISPLAY_READ_UNKNOWN3:
		return process_bafang_display_read_unknown3();
	case OPCODE_BAFANG_DISPLAY_READ_MOVING:
		return process_bafang_display_read_moving();
	}

	return DISCARD;
}

static int8_t try_process_bafang_write_request()
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



static int8_t process_read_fw_version()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart1_and_increment_checksum(OPCODE_READ_FW_VERSION, &checksum);
		write_uart1_and_increment_checksum(VERSION_MAJOR, &checksum);
		write_uart1_and_increment_checksum(VERSION_MINOR, &checksum);
		write_uart1_and_increment_checksum(VERSION_PATCH, &checksum);
		write_uart1_and_increment_checksum(CONFIG_VERSION, &checksum);
		uart1_write(checksum);
	}

	return 3;
}

static int8_t process_read_evtlog_enable()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart1_and_increment_checksum(OPCODE_READ_EVTLOG_ENABLE, &checksum);
		write_uart1_and_increment_checksum((uint8_t)eventlog_is_enabled(), &checksum);
		uart1_write(checksum);
	}

	return 3;
}

static int8_t process_read_config()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{
		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_READ, &checksum);
		write_uart1_and_increment_checksum(OPCODE_READ_CONFIG, &checksum);
		write_uart1_and_increment_checksum(CONFIG_VERSION, &checksum);
		write_uart1_and_increment_checksum(sizeof(config_t), &checksum);

		uint8_t* cfg = (uint8_t*)&g_config;
		for (uint8_t i = 0; i < sizeof(config_t); ++i)
		{
			write_uart1_and_increment_checksum(*(cfg + i), &checksum);
		}

		uart1_write(checksum);
	}

	return 3;
}


static int8_t process_write_evtlog_enable()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 3) == msgbuf[3])
	{
		eventlog_set_enabled((bool)msgbuf[2]);

		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart1_and_increment_checksum(OPCODE_WRITE_EVTLOG_ENABLE, &checksum);
		write_uart1_and_increment_checksum(msgbuf[2], &checksum);
		uart1_write(checksum);
	}

	return 4;
}

static int8_t process_write_config()
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

	if (compute_checksum(msgbuf, 4 + sizeof(config_t)) == msgbuf[4 + sizeof(config_t)] &&
		version == CONFIG_VERSION && length == sizeof(config_t))
	{	
		memcpy(&g_config, msgbuf + 4, sizeof(config_t));
		bool result = cfgstore_save();

		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart1_and_increment_checksum(OPCODE_WRITE_CONFIG, &checksum);
		write_uart1_and_increment_checksum(result, &checksum);
		uart1_write(checksum);		
	}

	return 4 + length + 1;
}

static int8_t process_write_reset_config()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf, 2) == msgbuf[2])
	{

		bool res = cfgstore_reset();

		uint8_t checksum = 0;
		write_uart1_and_increment_checksum(REQUEST_TYPE_WRITE, &checksum);
		write_uart1_and_increment_checksum(OPCODE_WRITE_RESET_CONFIG, &checksum);
		write_uart1_and_increment_checksum((uint8_t)res, &checksum);
		uart1_write(checksum);
	}

	return 3;
}


static int8_t process_bafang_display_read_status()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uart1_write(app_get_status_code());

	return 2;
}

static int8_t process_bafang_display_read_current()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t amp_x2 = (uint8_t)(motor_get_battery_current_x10() * 2) / 10;

	uart1_write(amp_x2);
	uart1_write(amp_x2); // checksum

	return 2;
}

static int8_t process_bafang_display_read_battery()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t value = motor_get_battery_voltage_x10() / 10;

	// should be in percent but can't be bottered to do SOC calculation.
	// return in volts instead, i.e. 57% on display will correspond to 57V.
	uart1_write(value);
	uart1_write(value); // checksum

	return 2;
}

static int8_t process_bafang_display_read_speed()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint16_t speed = speed_sensor_get_rpm_x10() / 10;
	uart1_write(speed >> 8);
	uart1_write(speed);
	uart1_write(0x20 + (speed >> 8) + speed); // weird checksum

	return 2;
}

static int8_t process_bafang_display_read_unknown1()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return 3;
}

static int8_t process_bafang_display_read_range()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uint8_t temp = app_get_motor_temperature();

	uart1_write(0x00);
	uart1_write(temp);
	uart1_write(temp); // checksum

	return 3;
}

static int8_t process_bafang_display_read_unknown2()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return 3;
}

static int8_t process_bafang_display_read_unknown3()
{
	if (msg_len < 3)
	{
		return KEEP;
	}

	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return 3;
}

static int8_t process_bafang_display_read_moving()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	uint8_t data = speed_sensor_is_moving() ? 0x30 : 0x31;
	uart1_write(data);
	uart1_write(data); // checksum

	return 2;
}


static int8_t process_bafang_display_write_pas()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	uint8_t level = ASSIST_0;

	switch (msgbuf[2])
	{
	case 0x00:
		level = ASSIST_0;
		break;
	case 0x01:
		level = ASSIST_1;
		break;
	case 0x0b:
		level = ASSIST_2;
		break;
	case 0x0c:
		level = ASSIST_3;
		break;
	case 0x0d:
		level = ASSIST_4;
		break;
	case 0x02:
		level = ASSIST_5;
		break;
	case 0x15:
		level = ASSIST_6;
		break;
	case 0x16:
		level = ASSIST_7;
		break;
	case 0x17:
		level = ASSIST_8;
		break;
	case 0x03:
		level = ASSIST_9;
		break;
	case 0x06:
		level = ASSIST_PUSH;
		break;
	}

	app_set_assist_level(level);

	return 4;
}

static int8_t process_bafang_display_write_mode()
{
	if (msg_len < 4)
	{
		return KEEP;
	}

	uint8_t mode = OPERATION_MODE_DEFAULT;

	switch (msgbuf[2])
	{
	case 0x02:
		mode = OPERATION_MODE_DEFAULT;
		break;
	case 0x04:
		mode = OPERATION_MODE_SPORT;
		break;
	}

	app_set_operation_mode(mode);

	return 4;
}

static int8_t process_bafang_display_write_lights()
{
	if (msg_len < 2)
	{
		return KEEP;
	}

	switch (msgbuf[2])
	{
	case 0xf0:
		app_set_lights(false);
		break;
	case 0xf1:
		app_set_lights(true);
		break;
	}

	return 2;
}

static int8_t process_bafang_display_write_speed_limit()
{
	if (msg_len < 5)
	{
		return KEEP;
	}

	if (compute_checksum(msgbuf + 2, 2) == msgbuf[4])
	{

		uint16_t value = ((msgbuf[2] << 8) | msgbuf[3]);
		app_set_wheel_max_speed_rpm(value);
	}

	return 5;
}
