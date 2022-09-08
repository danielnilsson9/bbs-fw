/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "stc15.h"
#include "interrupt.h"
#include <stdint.h>

void timers_init();

void timer0_init_system();
void timer0_init_sensors();

void timer1_init_uart1(uint32_t baudrate);
void timer2_init_uart2(uint32_t baudrate);

INTERRUPT_USING(isr_timer0, IRQ_TIMER0, 1);

#endif
