/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "watchdog.h"
#include "bbsx/stc15.h"

static bool triggered;

void watchdog_init()
{
	triggered = IS_BIT_SET(WDT_CONTR, 7);
	WDT_CONTR = 0x34; // Enable watchdog timer, pre-scaler 32 (625ms, 20MHz)
}

void watchdog_yeild()
{
	SET_BIT(WDT_CONTR, 4);
}

bool watchdog_triggered()
{
	return triggered;
}
