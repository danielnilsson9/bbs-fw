/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "uart.h"
#include "system.h"
#include "watchdog.h"
#include "bbsx/stc15.h"
#include "bbsx/uart_motor.h"
#include "bbsx/timers.h"
#include "bbsx/pins.h"

#include <stdint.h>

// NOTE:
// Variables located in __data are there for atomic access.

// UART1 (main)
#define RX1_BUFFER_SIZE			64
#define RX1_BUFFER_MASK			(RX1_BUFFER_SIZE - 1)

#define TX1_BUFFER_SIZE			32
#define TX1_BUFFER_MASK			(TX1_BUFFER_SIZE - 1)

static volatile __data uint8_t rx1_head;
static volatile __data uint8_t rx1_tail;
static volatile uint8_t rx1_buf[RX1_BUFFER_SIZE];
static volatile __data uint8_t tx1_head;
static volatile __data uint8_t tx1_tail;
static volatile __data uint8_t tx1_sending;
static volatile uint8_t tx1_buf[TX1_BUFFER_SIZE];

// UART2 (motor)
#define RX2_BUFFER_SIZE			16
#define RX2_BUFFER_MASK			(RX2_BUFFER_SIZE - 1)

#define TX2_BUFFER_SIZE			16
#define TX2_BUFFER_MASK			(TX2_BUFFER_SIZE - 1)

static volatile __data uint8_t rx2_head;
static volatile __data uint8_t rx2_tail;
static volatile uint8_t rx2_buf[RX2_BUFFER_SIZE];
static volatile __data uint8_t tx2_head;
static volatile __data uint8_t tx2_tail;
static volatile __data uint8_t tx2_sending;
static volatile uint8_t tx2_buf[TX2_BUFFER_SIZE];


void uart_open(uint32_t baudrate)
{
	rx1_head = 0;
	rx1_tail = 0;
	tx1_head = 0;
	tx1_tail = 0;
	tx1_sending = 0;

#if (GET_PORT_NUM(PIN_EXTERNAL_RX) == 3 && GET_PORT_NUM(PIN_EXTERNAL_TX) == 3)
	AUXR1 = (AUXR1 & 0x3f) | 0x00; // Keep UART1 on P3.0/P3.1
#else
	#error Unupported UART port configured.
#endif

	SET_PIN_QUASI(PIN_EXTERNAL_RX);
	SET_PIN_QUASI(PIN_EXTERNAL_TX);

	AUXR &= ~0x01; // Clock UART1 from T1
	PCON &= ~0x40; // Expose SM0 bit
	SM1 = 1; // UART 8-N-1
	SM0 = 0;
	SM2 = 0; // Point-to-point UART
	ES = 1; // Enable serial interrupt

	timer1_init_uart1(baudrate);

	REN = 1; // Rx enable
}

void uart_motor_open(uint32_t baudrate)
{
	rx2_head = 0;
	rx2_tail = 0;
	tx2_head = 0;
	tx2_tail = 0;
	tx2_sending = 0;

#if (GET_PORT_NUM(PIN_MOTOR_RX) == 1 && GET_PORT_NUM(PIN_MOTOR_TX) == 1)
	{
		P_SW2 = (P_SW2 & 0xfe) | 0x00; // Keep UART2 on P1.0/P1.1
	}
#else
	#error Unupported UART port configured.
#endif

	SET_PIN_QUASI(PIN_MOTOR_RX);
	SET_PIN_QUASI(PIN_MOTOR_TX);

	// UART 2 can only user timer 2
	S2CON &= ~(1 << 7); // UART 8-N-1
	S2CON &= ~(1 << 5); // Point-to-point UART
	IE2 |= (1 << 0); // Enable serial 2 interrupt

	timer2_init_uart2(baudrate);

	S2CON |= (1 << 4); // Rx enable
}

void uart_close()
{
	REN = 0;
	uart_flush();
	TR1 = 0;
}

void uart_motor_close()
{
	S2CON &= ~(1 << 4); // Rx disable
	uart_motor_flush();
	AUXR &= ~(1 << 4); // Stop timer 2
}


uint8_t uart_available()
{
	return (RX1_BUFFER_SIZE + rx1_head - rx1_tail) & RX1_BUFFER_MASK;
}

uint8_t uart_motor_available()
{
	return (RX2_BUFFER_SIZE + rx2_head - rx2_tail) & RX2_BUFFER_MASK;
}

uint8_t uart_read()
{
	uint8_t byte = rx1_buf[rx1_tail];
	rx1_tail = (rx1_tail + 1) & RX1_BUFFER_MASK;
	return byte;
}

uint8_t uart_motor_read()
{
	uint8_t byte = rx2_buf[rx2_tail];
	rx2_tail = (rx2_tail + 1) & RX2_BUFFER_MASK;
	return byte;
}

void uart_write(uint8_t byte)
{
	if (!tx1_sending)
	{
		tx1_sending = 1;
		SBUF = byte;

		return;
	}

	uint8_t i = (tx1_head + 1) & TX1_BUFFER_MASK;

	// wait for free space in buffer
	uint8_t prev_tail = tx1_tail;
	while (i == tx1_tail)
	{
		if (tx1_tail != prev_tail)
		{
			prev_tail = tx1_tail;
			watchdog_yeild();
		}
	}

	tx1_buf[tx1_head] = byte;
	tx1_head = i;

}

void uart_motor_write(uint8_t byte)
{
	if (!tx2_sending)
	{
		tx2_sending = 1;
		S2BUF = byte;

		return;
	}

	uint8_t i = (tx2_head + 1) & TX2_BUFFER_MASK;

	// wait for free space in buffer
	uint8_t prev_tail = tx1_tail;
	while (i == tx2_tail)
	{
		if (tx2_tail != prev_tail)
		{
			prev_tail = tx2_tail;
			watchdog_yeild();
		}
	}

	tx2_buf[tx2_head] = byte;
	tx2_head = i;
}

void uart_flush()
{
	while (tx1_sending);
}

void uart_motor_flush()
{
	while (tx2_sending);
}


INTERRUPT_USING(isr_uart1, IRQ_UART1, 3)
{
	if (RI) // rx interrupt
	{
		RI = 0;

		uint8_t c = SBUF;
		uint8_t i = (rx1_head + 1) & RX1_BUFFER_MASK;

		if (i != rx1_tail)
		{
			rx1_buf[rx1_head] = c;
			rx1_head = i;
		}
	}

	if (TI) // tx interrupt
	{
		TI = 0;

		if (tx1_head != tx1_tail)
		{
			tx1_sending = 1;

			SBUF = tx1_buf[tx1_tail];
			tx1_tail = (tx1_tail + 1) & TX1_BUFFER_MASK;
		}
		else
		{
			tx1_sending = 0;
		}
	}
}

INTERRUPT_USING(isr_uart2, IRQ_UART2, 3)
{
	if (S2CON & (1 << 0)) // rx interrupt
	{
		S2CON &= ~(1 << 0);

		uint8_t c = S2BUF;
		uint8_t i = (rx2_head + 1) & RX2_BUFFER_MASK;

		if (i != rx2_tail)
		{
			rx2_buf[rx2_head] = c;
			rx2_head = i;
		}
	}

	if (S2CON & (1 << 1)) // tx interrupt
	{
		S2CON &= ~(1 << 1);

		if (tx2_head != tx2_tail)
		{
			tx2_sending = 1;

			S2BUF = tx2_buf[tx2_tail];
			tx2_tail = (tx2_tail + 1) & TX2_BUFFER_MASK;
		}
		else
		{
			tx2_sending = 0;
		}
	}
}

