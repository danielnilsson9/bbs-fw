/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#include "watchdog.h"
#include "stc15.h"


void watchdog_init()
{
	WDT_CONTR |= 0x10; // Enable watchdog timer.
}

void watchdog_yeild()
{
	WDT_CONTR |= 0x80;
}

