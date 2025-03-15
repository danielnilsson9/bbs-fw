/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _BBSX_INTERRUPT_H_
#define _BBSX_INTERRUPT_H_

#include <stc12.h>

#define IRQ_TIMER0 1
#define IRQ_UART1  4
#define IRQ_UART2  8

INTERRUPT_USING(isr_timer0, IRQ_TIMER0, 1); // system.c
INTERRUPT_USING(isr_uart1, IRQ_UART1, 3);   // uart.c
INTERRUPT_USING(isr_uart2, IRQ_UART2, 3);   // uart.c

#endif
