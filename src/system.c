/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "system.h"
#include "watchdog.h"
#include "interrupt.h"

#define TIMER0_RELOAD	(65536 - CPU_FREQ / 1000) + 1

volatile uint32_t	_ms = 0;

void system_init()
{
	EA = 0; // disable interrupts

	TMOD = (TMOD & 0xf0) | 0x00; // Timer 0: 16-bit with autoreload
	AUXR |= 0x80; // Run timer 0 at CPU_FREQ

	TH0 = TIMER0_RELOAD >> 8;
	TL0 = TIMER0_RELOAD;

	EA = 1; // enable interrupts
	ET0 = 1; // enable timer0 interrupts
	TR0 = 1; // start timer 0
}

uint32_t system_ms()
{
	uint32_t val;
	ET0 = 0; // disable timer0 interrupts
	val = _ms;
	ET0 = 1;
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


INTERRUPT_USING(isr_timer0, TIMER0_IRQ, 2)
{
	_ms++;
}

