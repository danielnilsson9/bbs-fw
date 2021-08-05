/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "eventlog.h"
#include "stc15.h"
#include "uart.h"


static __xdata bool is_enabled;
static __xdata char buffer[256];


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

	uart1_write(0xee);
	uart1_write(evt);
}
void eventlog_write_data(uint8_t evt, int16_t data)
{
	if (!is_enabled)
	{
		return;
	}

	uart1_write(0xed);
	uart1_write(evt);
	uart1_write((uint8_t)(data >> 8));
	uart1_write((uint8_t)data);
}


