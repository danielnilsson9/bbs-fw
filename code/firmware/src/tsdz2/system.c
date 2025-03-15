/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#include "system.h"
#include "cpu.h"
#include "tsdz2/interrupt.h"
#include "tsdz2/timers.h"
#include "watchdog.h"

#include <stm8/stm8s.h>
#include <stm8/stm8s_clk.h>
#include <stm8/stm8s_tim3.h>

static volatile uint32_t _ms;

void system_init()
{
    CLK->CKDIVR = 0x00; // Set 16MHz
    while ((CLK->ICKR & CLK_ICKR_HSIRDY) == 0)
        ; // Wait for stable clock

    _ms = 0;

    // Setup timer3 as a ms counter
    timer3_init_system();

    enableInterrupts();
}

uint32_t system_ms()
{
    uint32_t val;
    uint8_t ier = TIM3->IER;

    TIM3->IER &= ~(TIM3_IT_UPDATE); // disable timer3 interrupt
    val = _ms;

    TIM3->IER = ier;

    return val;
}

void system_delay_ms(uint16_t ms)
{
    if (!ms)
    {
        return;
    }

    uint32_t end = system_ms() + ms;
    while (system_ms() != end)
    {
        watchdog_yeild();
    }
}

void isr_timer3_ovf(void) __interrupt(ITC_IRQ_TIM3_OVF)
{
    _ms++;

    // Clear interrupt pending bit
    TIM3->SR1 &= (uint8_t)(~TIM3_IT_UPDATE);
}
