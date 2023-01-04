/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _BBSX_UART_MOTOR_H_
#define _BBSX_UART_MOTOR_H_

#include "bbsx/stc15.h"
#include "bbsx/interrupt.h"

#include <stdint.h>

void uart_motor_open(uint32_t baudrate);
void uart_motor_close();

uint8_t uart_motor_available();
uint8_t uart_motor_read();

void uart_motor_write(uint8_t byte);
void uart_motor_flush();

#endif
