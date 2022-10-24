/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef  _TSDZ2_INTERRUPT_H_
#define _TSDZ2_INTERRUPT_H_
#include "intellisense.h"
#include "tsdz2/cpu.h"
#include "tsdz2/stm8s/stm8s_itc.h"

void isr_timer1_cmp(void) __interrupt(ITC_IRQ_TIM1_CAPCOM); // motor.c
void isr_timer3_ovf(void) __interrupt(ITC_IRQ_TIM3_OVF);	// system.c
void isr_timer4_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF);	// sensors.c

void isr_uart2_rx(void) __interrupt(ITC_IRQ_UART2_RX);		// uart.c
void isr_uart2_tx(void) __interrupt(ITC_IRQ_UART2_TX);		// uart.c

#endif
