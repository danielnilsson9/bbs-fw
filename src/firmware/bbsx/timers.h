/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _BBSX_TIMER_H_
#define _BBSX_TIMER_H_

#include "bbsx/stc15.h"
#include <stdint.h>

void timer0_init_system();
void timer0_init_sensors();

void timer1_init_uart1(uint32_t baudrate);
void timer2_init_uart2(uint32_t baudrate);

#endif
