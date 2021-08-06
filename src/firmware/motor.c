/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "motor.h"
#include "system.h"
#include "uart.h"
#include "pins.h"
#include "eventlog.h"

#include <stdbool.h>

#define OPCODE_LVC				0x60
#define OPCODE_MAX_CURRENT		0x61
#define OPCODE_TARGET_SPEED		0x63
#define OPCODE_TARGET_CURRENT	0x64
#define OPCODE_HELLO			0x67
#define OPCODE_UNKNOWN1			0x68
#define OPCODE_UNKNOWN2			0x69
#define OPCODE_UNKNOWN3			0x6A
#define OPCODE_UNKNOWN4			0x6B
#define OPCODE_UNKNOWN5			0x6C
#define OPCODE_UNKNOWN5			0x6C
#define OPCODE_UNKNOWN6			0x6D
#define OPCODE_UNKNOWN7			0x6E

#define OPCODE_READ_STATUS		0x40
#define OPCODE_READ_CURRENT		0x41
#define OPCODE_READ_VOLTAGE		0x42

#define READ_TIMEOUT			100


static __xdata uint8_t is_connected;
static __xdata uint8_t msgbuf[8];

static __xdata uint32_t next_send_ms;
static __xdata uint32_t next_read_ms;

static __xdata bool target_speed_changed;
static __xdata uint8_t target_speed;

static __xdata bool target_current_changed;
static __xdata uint8_t target_current;

static __xdata uint8_t lvc_volt_x10;

static __xdata uint16_t status_flags;
static __xdata uint16_t battery_volt_x10;
static __xdata uint16_t battery_amp_x10;


static uint8_t compute_checksum(uint8_t* msg, uint8_t len);
static void send_request(uint8_t opcode, uint16_t data);
static int read_response(uint8_t opcode, uint16_t* out_data);
static int connect();
static int configure(uint16_t max_current_mA, uint8_t lvc_V);
static void read_status();


void motor_init(__xdata uint16_t max_current_mA, __xdata uint8_t lvc_V)
{
	is_connected = 0;
	next_send_ms = 0;
	next_read_ms = 0;
	target_speed_changed = false;
	target_speed = 0;
	target_current_changed = false;
	target_current = 0;
	lvc_volt_x10 = (uint16_t)lvc_V * 10;
	status_flags = 0;
	battery_volt_x10 = 0;
	battery_amp_x10 = 0;

	SET_PIN_OUTPUT(PIN_MOTOR_POWER_ENABLE);
	SET_PIN_OUTPUT(PIN_MOTOR_CONTROL_ENABLE);
	SET_PIN_OUTPUT(PIN_MOTOR_EXTRA);

	SET_PIN_LOW(PIN_MOTOR_POWER_ENABLE);
	SET_PIN_HIGH(PIN_MOTOR_CONTROL_ENABLE);
	SET_PIN_HIGH(PIN_MOTOR_EXTRA);

	uart2_open(4800);

	// Extra wait for other MCU to power on
	system_delay_ms(100);

	if (connect() && configure(max_current_mA, lvc_V))
	{
		is_connected = 1;

		eventlog_write(EVT_MSG_MOTOR_INIT_OK);

		motor_set_target_speed(0);
		motor_set_target_current(0);
		target_current_changed = true;
		target_speed_changed = true;
	}
	else
	{
		eventlog_write(EVT_ERROR_INIT_MOTOR);
	}
}

void motor_process()
{
	if (!is_connected)
	{
		return;
	}

	uint32_t ms = system_ms();

	if (ms > next_send_ms)
	{
		next_send_ms = ms + 64;

		if (target_speed_changed)
		{
			send_request(OPCODE_TARGET_SPEED, target_speed);
			if (read_response(OPCODE_TARGET_SPEED, 0))
			{
				target_speed_changed = false;
				eventlog_write_data(EVT_DATA_TARGET_SPEED, target_speed);
			}
			else
			{
				eventlog_write(EVT_ERROR_CHANGE_TARGET_SPEED);
			}
		}

		if (target_current_changed)
		{
			send_request(OPCODE_TARGET_CURRENT, target_current);
			if (read_response(OPCODE_TARGET_CURRENT, 0))
			{
				target_current_changed = false;
				eventlog_write_data(EVT_DATA_TARGET_CURRENT, target_current);
			}
			else
			{
				eventlog_write(EVT_ERROR_CHANGE_TARGET_CURRENT);
			}
		}
	}

	if (system_ms() > next_read_ms)
	{
		next_read_ms = system_ms() + 150;
		read_status();
	}
}


void motor_enable()
{
	SET_PIN_HIGH(PIN_MOTOR_POWER_ENABLE);
}

void motor_disable()
{
	SET_PIN_LOW(PIN_MOTOR_POWER_ENABLE);
}

__xdata uint16_t motor_status()
{
	return status_flags;
}


void motor_set_target_speed(uint8_t value)
{
	if (target_speed != value)
	{
		target_speed = value;
		target_speed_changed = true;
	}
}

void motor_set_target_current(uint8_t percent)
{
	if (percent > 100)
	{
		percent = 100;
	}

	if (target_current != percent)
	{
		target_current = percent;
		target_current_changed = true;
	}
}


__xdata uint16_t motor_get_battery_lvc_x10()
{
	return lvc_volt_x10;
}

__xdata uint16_t motor_get_battery_current_x10()
{
	return battery_amp_x10;
}

__xdata uint16_t motor_get_battery_voltage_x10()
{
	return battery_volt_x10;
}



static uint8_t compute_checksum(uint8_t* msg, uint8_t len)
{
	uint8_t checksum = 0;
	for (int i = 0; i < len; ++i)
	{
		checksum += *(msg + i);
	}

	return checksum;
}

static void send_request(uint8_t opcode, uint16_t data)
{
	uint8_t idx = 0;

	msgbuf[idx++] = 0xaa; // start of message
	msgbuf[idx++] = opcode;

	if (opcode == OPCODE_LVC)
	{
		msgbuf[idx++] = data >> 8;
		msgbuf[idx++] = data;
	}
	else if (opcode != OPCODE_READ_STATUS && opcode != OPCODE_READ_CURRENT && opcode != OPCODE_READ_VOLTAGE)
	{
		msgbuf[idx++] = data;
	}

	uint8_t checksum = compute_checksum(msgbuf + 1, idx - 1);
	msgbuf[idx++] = checksum;

	// empty rx buffer
	while (uart2_available()) uart2_read();

	for (uint8_t i = 0; i < idx; ++i)
	{
		uart2_write(msgbuf[i]);
	}

	uart2_flush();
}

static int read_response(uint8_t opcode, uint16_t* out_data)
{
	uint8_t read = 0;
	uint32_t end = system_ms() + READ_TIMEOUT;

	uint8_t len = (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE) ? 5 : 4;

	uint8_t i = 0;
	while (i < len && system_ms() < end)
	{
		if (uart2_available())
		{
			msgbuf[i++] = uart2_read();
		}
	}

	// :TODO: delay between subsequent requests, handle in som other way...
	system_delay_ms(4);

	if (i == len && msgbuf[1] == opcode)
	{
		uint8_t checksum = compute_checksum(&msgbuf[1], (uint8_t)(i - 2));
		if (checksum == msgbuf[i - 1])
		{
			if (out_data != 0)
			{
				if (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE)
				{
					*out_data = msgbuf[2] << 8 | msgbuf[3];
				}
				else
				{
					*out_data = msgbuf[2];
				}
			}

			return 1;
		}

		return 0; // failed to verify message
	}

	// read failure
	return 0;
}


static int connect()
{
	for (int i = 0; i < 10; ++i)
	{
		send_request(OPCODE_HELLO, 0x00);

		if (read_response(OPCODE_HELLO, 0))
		{
			return 1;
		}
		else
		{
			system_delay_ms(1000);
		}
	}

	return 0;
}

static int configure(uint16_t max_current_mA, uint8_t lvc_V)
{
	uint16_t tmp = 0;

	send_request(OPCODE_UNKNOWN1, 0x5a);
	if (!read_response(OPCODE_UNKNOWN1, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN2, 0x11);
	if (!read_response(OPCODE_UNKNOWN2, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN3, 0x78);
	if (!read_response(OPCODE_UNKNOWN3, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN4, 0x64);
	if (!read_response(OPCODE_UNKNOWN4, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN5, 0x50);
	if (!read_response(OPCODE_UNKNOWN5, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN6, 0x46);
	if (!read_response(OPCODE_UNKNOWN6, 0))
	{
		return 0;
	}

	send_request(OPCODE_UNKNOWN7, 0x0c);
	if (!read_response(OPCODE_UNKNOWN7, 0))
	{
		return 0;
	}

	send_request(OPCODE_LVC, lvc_V * 14);
	if (!read_response(OPCODE_LVC, 0))
	{
		return 0;
	}

	tmp = (uint16_t)((max_current_mA * 69UL) / 10000UL);
	if (tmp > 255)
	{
		tmp = 255;
	}
	eventlog_write_data(EVT_DATA_MAX_CURRENT_ADC_REQUEST, tmp);
	send_request(OPCODE_MAX_CURRENT, tmp);
	if (!read_response(OPCODE_MAX_CURRENT, &tmp))
	{
		return 0;
	}
	else
	{
		eventlog_write_data(EVT_DATA_MAX_CURRENT_ADC_RESPONSE, tmp);
	}

	return 1;
}

static void read_status()
{
	uint16_t value = 0;

	send_request(OPCODE_READ_STATUS, 0);
	if (read_response(OPCODE_READ_STATUS, &value))
	{
		if (value != status_flags)
		{
			status_flags = value;
			eventlog_write_data(EVT_DATA_MOTOR_STATUS, status_flags);
		}	
	}

	send_request(OPCODE_READ_CURRENT, 0);
	if (read_response(OPCODE_READ_CURRENT, &value))
	{
		battery_amp_x10 = (value * 100) / 69;
	}

	send_request(OPCODE_READ_VOLTAGE, 0);
	if (read_response(OPCODE_READ_VOLTAGE, &value))
	{
		battery_volt_x10 = (value * 10) / 14;
	}
}
