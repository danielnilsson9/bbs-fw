/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "timers.h"
#include "cpu.h"
#include "tsdz2/stm8s/stm8s.h"
#include "tsdz2/stm8s/stm8s_clk.h"
#include "tsdz2/stm8s/stm8s_tim1.h"
#include "tsdz2/stm8s/stm8s_tim2.h"
#include "tsdz2/stm8s/stm8s_tim3.h"
#include "tsdz2/stm8s/stm8s_tim4.h"

#define TIM1_AUTO_RELOAD_PERIOD			511
#define TIM2_AUTO_RELOAD_PERIOD			159		// 20us
#define TIM3_AUTO_RELOAD_PERIOD			15999	// 1ms
#define TIM4_AUTO_RELOAD_PERIOD			99		// 100us


void timers_init()
{
	// nothing to do here
}


void timer1_init_motor_pwm()
{
	CLK->PCKENR1 |= CLK_PCKENR1_TIM1;

	// prescaler
	TIM1->PSCRH = 0;
	TIM1->PSCRL = 0;

	// auto reload
	// clock = 16MHz, counter period = 1024, PWM freq = 16MHz / 1024 = 15.625MHz
	// (BUT PWM center aligned mode needs double frequency)
	TIM1->ARRH = (uint8_t)(TIM1_AUTO_RELOAD_PERIOD >> 8);
	TIM1->ARRL = (uint8_t)TIM1_AUTO_RELOAD_PERIOD;

	TIM1->CR1 |= TIM1_COUNTERMODE_CENTERALIGNED1;
	TIM1->RCR = 1;

	// OC1
	TIM1->CCER1 |= (uint8_t)(
		(uint8_t)(TIM1_OUTPUTSTATE_DISABLE & TIM1_CCER1_CC1E) |
		(uint8_t)(TIM1_OUTPUTNSTATE_DISABLE & TIM1_CCER1_CC1NE) |
		(uint8_t)(TIM1_OCPOLARITY_HIGH & TIM1_CCER1_CC1P) |
		(uint8_t)(TIM1_OCNPOLARITY_HIGH & TIM1_CCER1_CC1NP)
	);

	TIM1->CCMR1 |= TIM1_OCMODE_PWM1;

	TIM1->OISR |= (uint8_t)(
		(uint8_t)(TIM1_OCIDLESTATE_RESET & TIM1_OISR_OIS1) |
		(uint8_t)(TIM1_OCNIDLESTATE_SET & TIM1_OISR_OIS1N)
	);

	TIM1->CCR1H = 0;
	TIM1->CCR1L = 255;


	// OC2
	TIM1->CCER1 |= (uint8_t)(
		(uint8_t)(TIM1_OUTPUTSTATE_DISABLE & TIM1_CCER1_CC2E) |
		(uint8_t)(TIM1_OUTPUTNSTATE_DISABLE & TIM1_CCER1_CC2NE) |
		(uint8_t)(TIM1_OCPOLARITY_HIGH & TIM1_CCER1_CC2P) |
		(uint8_t)(TIM1_OCNPOLARITY_HIGH & TIM1_CCER1_CC2NP)
		);

	TIM1->CCMR2 |= TIM1_OCMODE_PWM1;

	TIM1->OISR |= (uint8_t)(
		(uint8_t)(TIM1_OCIDLESTATE_RESET & TIM1_OISR_OIS2) |
		(uint8_t)(TIM1_OCNIDLESTATE_SET & TIM1_OISR_OIS2N)
		);

	TIM1->CCR2H = 0;
	TIM1->CCR2L = 255;

	// OC3
	TIM1->CCER2 |= (uint8_t)(
		(uint8_t)(TIM1_OUTPUTSTATE_DISABLE & TIM1_CCER2_CC3E) |
		(uint8_t)(TIM1_OUTPUTNSTATE_DISABLE & TIM1_CCER2_CC3NE) |
		(uint8_t)(TIM1_OCPOLARITY_HIGH & TIM1_CCER2_CC3P) |
		(uint8_t)(TIM1_OCNPOLARITY_HIGH & TIM1_CCER2_CC3NP)
		);

	TIM1->CCMR3 |= TIM1_OCMODE_PWM1;

	TIM1->OISR |= (uint8_t)(
		(uint8_t)(TIM1_OCIDLESTATE_RESET & TIM1_OISR_OIS3) |
		(uint8_t)(TIM1_OCNIDLESTATE_SET & TIM1_OISR_OIS3N)
		);

	TIM1->CCR3H = 0;
	TIM1->CCR3L = 255;

	// OC4
	// Used for to fire interrupt at a specific time (middle of DC link current pulses)
	// and is always syncronized with PWM

	TIM1->CCER2 |= (uint8_t)(
		(uint8_t)(TIM1_OUTPUTSTATE_DISABLE & TIM1_CCER2_CC4E) |
		(uint8_t)(TIM1_OCPOLARITY_HIGH & TIM1_CCER2_CC4P)
	);

	TIM1->OISR &= (uint8_t)(~TIM1_OISR_OIS4);

	// timming for interrupt firing (hand adjusted)
	const uint16_t Timing = 285;

	TIM1->CCR4H = (uint8_t)(Timing >> 8);
	TIM1->CCR4L = (uint8_t)Timing;

	// hardware needs a dead time of 1us
	//	16, // DTG = 0; dead time in 62.5 ns steps; 1us/62.5ns = 16
	TIM1->DTR = (uint8_t)16;

	TIM1->BKR = (uint8_t)(
		TIM1_OSSISTATE_ENABLE |
		TIM1_LOCKLEVEL_OFF |
		TIM1_BREAK_DISABLE |
		TIM1_BREAKPOLARITY_LOW |
		TIM1_AUTOMATICOUTPUT_DISABLE
	);

	// enable cc4 interrupt
	TIM1->IER |= TIM1_IT_CC4;

	// enable timer
	TIM1->CR1 |= TIM1_CR1_CEN;

	TIM1->BKR |= TIM1_BKR_MOE;
}

void timer2_init_torque_sensor_pwm()
{
	// Timer2 is used to create the pulse signal for excitation of the torque sensor circuit
	// Timer2 clock = 16MHz; target: 20us period --> 50khz
	// counter period = (1 / (16000000 / prescaler)) * (159 + 1) = 20us

	// set period
	TIM2->PSCR = TIM2_PRESCALER_2;
	TIM2->ARRH = (uint8_t)(TIM2_AUTO_RELOAD_PERIOD >> 8);
	TIM2->ARRL = (uint8_t)(TIM2_AUTO_RELOAD_PERIOD);

	// pulse of 2us
	TIM2->CCER1 |= TIM2_CCER1_CC2E; // output enable
	TIM2->CCMR2 |= TIM2_OCMODE_PWM1;
	TIM2->CCR2H = 0;
	TIM2->CCR2L = 16;

	// enable
	TIM2->CCMR2 |= TIM2_CCMR_OCxPE;
	TIM2->CR1 |= TIM2_CR1_ARPE;
	TIM2->CR1 |= TIM2_CR1_CEN;
}

void timer3_init_system()
{
	// enable timer3 clock source
	CLK->PCKENR1 |= CLK_PCKENR1_TIM3;

	// set period
	TIM3->PSCR = TIM3_PRESCALER_1;
	TIM3->ARRH = (uint8_t)(TIM3_AUTO_RELOAD_PERIOD >> 8);
	TIM3->ARRL = (uint8_t)(TIM3_AUTO_RELOAD_PERIOD);

	// clear counter
	TIM3->CNTRH = 0;
	TIM3->CNTRL = 0;

	// enable TIM3 interrupt
	TIM3->IER |= TIM3_IT_UPDATE;

	// clear interrupt pending bit
	TIM3->SR1 &= ~TIM3_IT_UPDATE;

	// TIM3 enable
	TIM3->CR1 |= TIM3_CR1_CEN;
}

void timer4_init_sensors()
{
	// enable timer4 clock source
	CLK->PCKENR1 |= CLK_PCKENR1_TIM4;

	// set period
	TIM4->PSCR = TIM4_PRESCALER_16;
	TIM4->ARR = TIM4_AUTO_RELOAD_PERIOD;

	// clear counter
	TIM4->CNTR = 0;

	// enable TIM4 interrupt
	TIM4->IER |= TIM4_IT_UPDATE;

	// clear interrupt pending bit
	TIM4->SR1 &= ~TIM4_IT_UPDATE;

	// TIM4 enable
	TIM4->CR1 |= TIM4_CR1_CEN;
}
