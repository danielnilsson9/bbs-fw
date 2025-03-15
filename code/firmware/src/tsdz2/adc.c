/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#include <stdint.h>

#include "adc.h"
#include "tsdz2/interrupt.h"
#include "tsdz2/pins.h"
#include "tsdz2/stm8.h"

#include <stm8/stm8s.h>
#include <stm8/stm8s_adc1.h>

static volatile uint8_t adc_throttle;
static volatile uint16_t adc_battery_voltage;
static volatile uint16_t adc_torque;

// cached variables read from voltatile uint16_t vars while ADC1 interrupt disabled
static uint16_t adc_battery_voltage_cache;
static uint16_t adc_torque_cache;

void adc_init()
{
    SET_PIN_INPUT(PIN_BATTERY_CURRENT);
    SET_PIN_INPUT(PIN_BATTERY_VOLTAGE);
    SET_PIN_INPUT(PIN_THROTTLE);
    SET_PIN_INPUT(PIN_TORQUE_SENSOR);

    // NOTE:
    // adc configuration (except ADC1->CR1) is overwritten in motor.c/isr_timer1_cmp
    // which triggeres the conversion.
    //
    // The motor control interrupt routines performs single mode
    // adc conversion of battery current, reads the result and
    // then starts buffered scan mode conversion of all adc channels
    // with end of conversion interrupt enabled which is handled here.

    ADC1->CR1 = ADC1_PRESSEL_FCPU_D2;
    ADC1->CR2 = ADC1_ALIGN_LEFT;

    // channel (none)
    ADC1->CSR = 0x00;

    // schmittrig disable all
    ADC1->TDRL |= (uint8_t)0xFF;
    ADC1->TDRH |= (uint8_t)0xFF;

    // Enable the ADC1 peripheral
    ADC1->CR1 |= ADC1_CR1_ADON;
}

void adc_process()
{
    // Have to disable interrupts globally since ADC1->CSR register
    // is manipulated from motor control isr. Very short time, should have no effect.
    disableInterrupts();
    adc_battery_voltage_cache = adc_battery_voltage; // adc_battery_voltage;
    adc_torque_cache = adc_torque;
    enableInterrupts();
}

uint8_t adc_get_throttle()
{
    // atomic read
    return adc_throttle;
}

uint16_t adc_get_torque()
{
    // 10 bit resolution
    return adc_torque_cache;
}

uint16_t adc_get_temperature_contr()
{
    return 0;
}

uint16_t adc_get_temperature_motor()
{
    return 0;
}

uint16_t adc_get_battery_voltage()
{
    return adc_battery_voltage_cache;
}

void isr_adc1(void) __interrupt(ITC_IRQ_ADC1)
{
    if (ADC1->CSR & ADC1_CSR_EOC)
    {
        // all adc channels converted, data available in buffers

        // clear EOC and disable EOC interrupt
        ADC1->CSR = 0x00;

        // scan mode reads are setup to be left aligned in motor isr

        // update cached values
        adc_throttle = ADC1->DB7RH; // only 8bit resolution used

        // must read in high -> low order according to data sheet
        uint8_t high, low;

        // read torque
        high = ADC1->DB4RH;
        low = ADC1->DB4RL;
        adc_torque = (uint16_t)high << 2 | low;

        // read battery voltage
        high = ADC1->DB6RH;
        low = ADC1->DB6RL;
        adc_battery_voltage = (uint16_t)high << 2 | low;
    }
}
