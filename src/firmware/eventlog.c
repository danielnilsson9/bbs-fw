/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "eventlog.h"
#include "uart.h"

static bool is_enabled;

void eventlog_init(bool enabled)
{
	is_enabled = enabled;
}

bool eventlog_is_enabled()
{
	return is_enabled;
}

void eventlog_set_enabled(bool enabled)
{
	is_enabled = enabled;
}

void eventlog_write(uint8_t evt)
{
	if (!is_enabled)
	{
		return;
	}

	uart_write(0xee);
	uart_write(evt);
	uart_write((uint8_t)0xee + evt);
}
void eventlog_write_data(uint8_t evt, int16_t data)
{
	if (!is_enabled)
	{
		return;
	}

	uint8_t checksum = 0;

	uart_write(0xed); checksum += (uint8_t)0xed;
	uart_write(evt); checksum += evt;
	uart_write((uint8_t)(data >> 8)); checksum += (uint8_t)(data >> 8);
	uart_write((uint8_t)data); checksum += (uint8_t)data;
	uart_write(checksum);
}
