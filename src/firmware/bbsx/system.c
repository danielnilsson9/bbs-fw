/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "system.h"
#include "watchdog.h"
#include "timers.h"
#include "bbsx/stc15.h"

static volatile uint32_t	_ms;
static volatile uint8_t		_x100us;

void system_init()
{
	_ms = 0;
	_x100us = 0;

	// Wait for stable voltage (above lvd)
	while (IS_BIT_SET(PCON, 5))
	{
		CLEAR_BIT(PCON, 5);
	}

	timer0_init_system();
}

uint32_t system_ms()
{
	uint32_t val;
	uint8_t et0 = ET0;
	ET0 = 0; // disable timer0 interrupts
	val = _ms;
	ET0 = et0;
	return val;
}

void system_delay_ms(uint16_t ms)
{
	if (!ms)
	{
		return;
	}

	uint32_t end = system_ms() + ms;
	while (system_ms() != end)
	{
		watchdog_yeild();
	}
}

#pragma save  
#pragma nooverlay // See SDCC manual about function calls in ISR
void system_timer0_isr()
{
	_x100us++;
	if (_x100us == 10)
	{
		_x100us = 0;
		_ms++;
	}
}
#pragma restore
