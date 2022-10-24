/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

void uart_open(uint32_t baudrate);
void uart_close();

uint8_t uart_available();
uint8_t uart_read();

void uart_write(uint8_t byte);
void uart_flush();

#endif
