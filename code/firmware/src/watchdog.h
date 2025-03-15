/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

// #include "intellisense.h"
#include <stdbool.h>
#include <stdint.h>

void watchdog_init();
void watchdog_yeild();

bool watchdog_triggered();

#endif
