/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef  _BBSX_INTERRUPT_H_
#define _BBSX_INTERRUPT_H_

#include <stc12.h>
#include "intellisense.h"

#define IRQ_TIMER0		1
#define IRQ_UART1		4
#define IRQ_UART2		8
#define IRQ_TIMER4		20

INTERRUPT_USING(isr_timer0, IRQ_TIMER0, 1);		// system.c
INTERRUPT_USING(isr_uart1, IRQ_UART1, 3);		// uart.c
INTERRUPT_USING(isr_uart2, IRQ_UART2, 3);		// uart.c

#endif
