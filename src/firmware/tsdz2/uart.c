/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "uart.h"
#include "interrupt.h"

#include <stdint.h>


static volatile uint8_t rx1_head;
static volatile uint8_t rx1_tail;
static volatile uint8_t rx1_buf[256];
static volatile uint8_t tx1_head;
static volatile uint8_t tx1_tail;
static volatile uint8_t tx1_sending;
static volatile uint8_t tx1_buf[256];

void uart_open(uint32_t baudrate)
{
	rx1_head = 0;
	rx1_tail = 0;
	tx1_head = 0;
	tx1_tail = 0;
	tx1_sending = 0;

	// enable uart2 clock
	CLK->PCKENR1 |= CLK_PCKENR1_UART2;

	// default, 8bit, no parity, 1 stop bit etc
	UART2->CR1 = 0x00;
	UART2->CR2 = 0x00;
	UART2->CR3 = 0x00;

	// clear the LSB mantissa of UART2DIV
	UART2->BRR1 &= (uint8_t)(~UART2_BRR1_DIVM);
	// clear the MSB mantissa of UART2DIV
	UART2->BRR2 &= (uint8_t)(~UART2_BRR2_DIVM);
	// clear the fraction bits of UART2DIV
	UART2->BRR2 &= (uint8_t)(~UART2_BRR2_DIVF);

	// set the UART2 baudrate in BRR1 and BRR2 registers according to baudrate value
	uint32_t baud_mantissa = ((uint32_t)CPU_FREQ / (baudrate << 4));
	uint32_t baud_mantissa100 = (((uint32_t)CPU_FREQ * 100) / (baudrate << 4));

	uint8_t BRR2_1 = (uint8_t)((uint8_t)(((baud_mantissa100 - (baud_mantissa * 100)) << 4) / 100) & (uint8_t)0x0F);
	uint8_t BRR2_2 = (uint8_t)((baud_mantissa >> 4) & (uint8_t)0xF0);

	UART2->BRR2 = (uint8_t)(BRR2_1 | BRR2_2);
	UART2->BRR1 = (uint8_t)baud_mantissa;

	// enable rx and tx
	UART2->CR2 |= UART2_CR2_TEN;
	UART2->CR2 |= UART2_CR2_REN;

	// clear rx and tx interrupt flags
	UART2->SR &= ~UART2_SR_RXNE;

	// enable rx interrupts
	UART2->CR2 |= UART2_CR2_RIEN;
}

void uart_close()
{
	UART2->BRR2 = 0x00;
	UART2->BRR1 = 0x00;

	UART2->CR1 = 0x00;
	UART2->CR2 = 0x00;
	UART2->CR3 = 0x00;
}

uint8_t uart_available()
{
	return (rx1_head - rx1_tail);
}

uint8_t uart_read()
{
	// Disable UART2 rx interrupt
	UART2->CR2 &= ~UART2_CR2_RIEN;

	uint8_t byte = rx1_buf[rx1_tail];
	rx1_tail = (rx1_tail + 1);

	// Enable UART1 interrupt
	UART2->CR2 |= UART2_CR2_RIEN;

	return byte;
}

void uart_write(uint8_t byte)
{
	if (tx1_sending)
	{
		// wait for free space in buffer
		while ((tx1_head + 1) == tx1_tail);

		tx1_buf[tx1_head] = byte;
		tx1_head = (tx1_head + 1);
	}
	else
	{
		tx1_sending = 1;		
		UART2->DR = byte;
		UART2->CR2 |= UART2_CR2_TIEN; // enable tx done interrupt
	}
}

void uart_flush()
{
	while (tx1_sending);
}



void isr_uart2_rx(void) __interrupt(ITC_IRQ_UART2_RX)
{
	if (UART2->SR & UART2_SR_RXNE)
	{
		if (rx1_head != (rx1_tail - 1))
		{
			rx1_buf[rx1_head] = UART2->DR;
			rx1_head = (rx1_head + 1);
		}
		else
		{
			// buffer full, discard data
			UART2->SR &= ~UART2_SR_RXNE;
		}
	}
}

void isr_uart2_tx(void) __interrupt(ITC_IRQ_UART2_TX)
{
	if (UART2->SR & UART2_SR_TXE)
	{
		if (tx1_head != tx1_tail)
		{
			tx1_sending = 1;
			UART2->DR = tx1_buf[tx1_tail];
			tx1_tail = (tx1_tail + 1);
		}
		else
		{
			tx1_sending = 0;
			// no more data clear tx empty flag
			UART2->CR2 &= ~UART2_CR2_TIEN;
		}
	}
}
