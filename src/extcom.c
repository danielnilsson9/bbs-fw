/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "extcom.h"
#include "stc15.h"
#include "uart.h"
#include "system.h"
#include "sensors.h"
#include "app.h"

#include <stdint.h>
#include <stdbool.h>

#define KEEP		0
#define DISCARD		1
#define COMPLETE	2



#define BUFFER_SIZE 128

#define REQUEST_TYPE_READ						0x01
#define REQUEST_TYPE_WRITE						0x02

#define REQUEST_TYPE_BAFANG_READ				0x11
#define REQUEST_TYPE_BAFANG_WRITE				0x16


// Firmware config tool communication



// Bafang display communication
#define OPCODE_BAFANG_DISPLAY_READ_STATUS		0x08
#define OPCODE_BAFANG_DISPLAY_READ_POWER		0x0a
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



static uint8_t idx = 0;
static uint8_t __xdata msgbuf[128];
static uint32_t last_recv = 0;


static uint8_t try_process_request();
static uint8_t try_process_read_request();
static uint8_t try_process_write_request();
static uint8_t try_process_bafang_read_request();
static uint8_t try_process_bafang_write_request();



static uint8_t process_bafang_display_read_status();
static uint8_t process_bafang_display_read_power();
static uint8_t process_bafang_display_read_battery();
static uint8_t process_bafang_display_read_speed();
static uint8_t process_bafang_display_read_unknown1();
static uint8_t process_bafang_display_read_range();
static uint8_t process_bafang_display_read_unknown2();
static uint8_t process_bafang_display_read_unknown3();
static uint8_t process_bafang_display_read_moving();

static uint8_t process_bafang_display_write_pas();
static uint8_t process_bafang_display_write_mode();
static uint8_t process_bafang_display_write_lights();
static uint8_t process_bafang_display_write_speed_limit();

void extcom_init()
{
	// bafang standard baudrate
	uart1_open(1200);
}

void extcom_process()
{
	uint32_t now = system_ms();

	while (uart1_available())
	{
		if (idx == BUFFER_SIZE)
		{
			// communication error, reset
			idx = 0;
			while (uart1_available()) uart1_read();
		}
		else
		{
			msgbuf[idx++] = uart1_read();
			last_recv = now;
		}	
	}

	if (idx > 0 && now - last_recv > 100)
	{
		// communication error, reset
		idx = 0;
	}

	if (try_process_request() != KEEP)
	{
		idx = 0;
		last_recv = 0;
	}
}



static uint8_t try_process_request()
{
	if (idx == 0)
	{
		return;
	}

	switch (msgbuf[0])
	{
	case REQUEST_TYPE_READ:
		return try_process_read_request();
	case REQUEST_TYPE_WRITE:
		return try_process_bafang_write_request();
	case REQUEST_TYPE_BAFANG_READ:
		return try_process_bafang_read_request();
		break;
	case REQUEST_TYPE_BAFANG_WRITE:
		return try_process_bafang_write_request();
	}

	return DISCARD; // unknown message
}

static uint8_t try_process_read_request()
{
	return DISCARD;
}

static uint8_t try_process_write_request()
{

	return DISCARD;
}

static uint8_t try_process_bafang_read_request()
{
	switch (msgbuf[1])
	{
	case OPCODE_BAFANG_DISPLAY_READ_STATUS:
		return process_bafang_display_read_status();
	case OPCODE_BAFANG_DISPLAY_READ_POWER:
		return process_bafang_display_read_power();
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

static uint8_t try_process_bafang_write_request()
{
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



static uint8_t process_bafang_display_read_status()
{
	uart1_write(0x00); // All ok :TODO: implement bafang error codes
	// 0x01 = pedaling
	// 0x03 = braking
	// error codes...

	return COMPLETE;
}

static uint8_t process_bafang_display_read_power()
{
	uart1_write(0);
	uart1_write(0); // checksum
	// :TODO: get current from motor and compute checksum (excl op)

	return COMPLETE;
}

static uint8_t process_bafang_display_read_battery()
{
	uart1_write(50); // 50%
	uart1_write(50); // checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_speed()
{
	uint16_t speed = speed_sensor_get_ticks_per_minute();
	uart1_write(speed >> 8);
	uart1_write(speed);
	uart1_write(0x20 + (speed >> 8) + speed); // weird checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_unknown1()
{
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_range()
{
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_unknown2()
{
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_unknown3()
{
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00);
	uart1_write(0x00); // checksum

	return COMPLETE;
}

static uint8_t process_bafang_display_read_moving()
{
	uint8_t data = speed_sensor_is_moving() ? 0x30 : 0x31;
	uart1_write(data);
	uart1_write(data); // checksum

	return COMPLETE;
}


static uint8_t process_bafang_display_write_pas()
{
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
	default:
		return DISCARD;
	}

	app_set_assist_level(level);

	return COMPLETE;
}

static uint8_t process_bafang_display_write_mode()
{
	uint8_t mode = MODE_SPORT;

	switch (msgbuf[2])
	{
	case 0x02:
		mode = MODE_ECO;
		break;
	case 0x04:
		mode = MODE_SPORT;
		break;
	default:
		return DISCARD;
	}

	app_set_operation_mode(mode);

	return KEEP;
}

static uint8_t process_bafang_display_write_lights()
{
	switch (msgbuf[2])
	{
	case 0xf0:
		app_set_lights(false);
		break;
	case 0xf1:
		app_set_lights(true);
		break;
	default:
		return DISCARD;
	}

	return KEEP;
}

static uint8_t process_bafang_display_write_speed_limit()
{
	uint8_t chk = 0;
	chk += msgbuf[2];
	chk += msgbuf[3];
	if (chk != msgbuf[4])
	{
		return DISCARD;
	}

	uint16_t value = ((msgbuf[2] << 8) | msgbuf[3]);
	app_set_wheel_max_speed_ppm(value);

	return COMPLETE;
}
