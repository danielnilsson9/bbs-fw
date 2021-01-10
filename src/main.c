/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "stc15.h"

#include "system.h"
#include "watchdog.h"
#include "motor.h"
#include "uart.h"

#define MAX_CURRENT_X1000		30000
#define LOW_VOLTAGE_CUTOFF		42


void main(void)
{
	system_init();
	watchdog_init();

	uart1_open(9600);

	motor_init(MAX_CURRENT_X1000, LOW_VOLTAGE_CUTOFF);

	// battery voltage as input
	//P1M0 &= ~(1 << 6);
	//P1M1 |= (1 << 6);

	//// brake as input
	//P2M0 &= ~(1 << 6);
	//P2M1 |= (1 << 4);

	//// hall sensor as inputs
	//P3M0 &= ~(1 << 4);
	//P3M1 |= (1 << 4);

	//P5M0 &= ~(1 << 0);
	//P5M1 |= (1 << 0);

	//P0M0 &= ~(1 << 6);
	//P0M1 |= (1 << 6);

	//// PAS inputs
	//P4M0 &= ~(1 << 5);
	//P4M1 |= (1 << 5);

	//P4M0 &= ~(1 << 6);
	//P4M1 |= (1 << 6);

	//motor_enable();
	//motor_set_target_current(8000);


	while (1)
	{
		motor_process();
		watchdog_yeild();
	}
}