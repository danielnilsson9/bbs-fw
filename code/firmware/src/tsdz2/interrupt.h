/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _TSDZ2_INTERRUPT_H_
#define _TSDZ2_INTERRUPT_H_

#include "tsdz2/cpu.h"
#include <stm8/stm8s_itc.h>

void isr_timer1_cmp(void) __interrupt(ITC_IRQ_TIM1_CAPCOM); // motor.c
void isr_timer3_ovf(void) __interrupt(ITC_IRQ_TIM3_OVF);    // system.c
void isr_timer4_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF);    // sensors.c

void isr_adc1(void) __interrupt(ITC_IRQ_ADC1); // adc.c

void isr_uart2_rx(void) __interrupt(ITC_IRQ_UART2_RX); // uart.c
void isr_uart2_tx(void) __interrupt(ITC_IRQ_UART2_TX); // uart.c

#endif
