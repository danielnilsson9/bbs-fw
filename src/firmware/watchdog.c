/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "watchdog.h"
#include "stc15.h"


void watchdog_init()
{
	WDT_CONTR = 0x34; // Enable watchdog timer, pre-scaler 32 (680ms)
}

void watchdog_yeild()
{
	SET_BIT(WDT_CONTR, 4);
}

