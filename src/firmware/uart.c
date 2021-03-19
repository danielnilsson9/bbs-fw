#include "uart.h"
#include "system.h"
#include <stdint.h>


// UART1
static volatile uint8_t rx1_head;
static volatile uint8_t rx1_tail;
static volatile uint8_t __xdata rx1_buf[256];
static volatile uint8_t tx1_head;
static volatile uint8_t tx1_tail;
static volatile uint8_t tx1_sending;
static volatile uint8_t __xdata tx1_buf[256];

// UART2
static volatile uint8_t rx2_head;
static volatile uint8_t rx2_tail;
static volatile uint8_t __xdata rx2_buf[256];
static volatile uint8_t tx2_head;
static volatile uint8_t tx2_tail;
static volatile uint8_t tx2_sending;
static volatile uint8_t __xdata tx2_buf[256];


void uart1_open(uint32_t baudrate)
{
	rx1_head = 0;
	rx1_tail = 0;
	tx1_head = 0;
	tx1_tail = 0;
	tx1_sending = 0;

	unsigned short reload = 65535 - CPU_FREQ / 4 / baudrate + 1;

	// Set up signal routing around UART1
	AUXR1 = (AUXR1 & 0x3f) | 0x00; // Keep UART1 on P3.0/P3.1
	P3M0 &= ~0x03; // Put pins P3.0/P3.1 into quasi bidirectional mode.
	P3M1 &= ~0x03;
	AUXR &= ~0x01; // Clock UART1 from T1
	PCON &= ~0x40; // Expose SM0 bit
	SM1 = 1; // UART 8-N-1
	SM0 = 0;
	SM2 = 0; // Point-to-point UART
	ES = 1; // Enable serial interrupt

	// Set up timer 1 for baudrate
	TMOD = (TMOD & 0x0f) | 0x00; // Run T1 in mode 0 (16-bit reload)
	AUXR |= 0x40; // Run T1 at CPU_FREQ
	TL1 = reload; // Set the reload value for given baudrate.
	TH1 = reload >> 8;
	ET1 = 0; // No interrupts from timer 1.
	TR1 = 1; // Start timer 1

	REN = 1; // Rx enable
}

void uart2_open(uint32_t baudrate)
{
	rx2_head = 0;
	rx2_tail = 0;
	tx2_head = 0;
	tx2_tail = 0;
	tx2_sending = 0;

	unsigned short reload = 65535 - CPU_FREQ / 4 / baudrate + 1;

	P_SW2 = (P_SW2 & 0xfe) | 0x00; // Keep UART2 on P1.0/P1.1
	P1M0 &= ~0x03; // Put pins P1.0/P1.1 into quasi bidirectional mode.
	P1M1 &= ~0x03;
	// UART 2 can only user timer 2
	S2CON &= ~(1 << 7); // UART 8-N-1
	S2CON &= ~(1 << 5); // Point-to-point UART
	IE2 |= (1 << 0); // Enable serial 2 interrupt

	// Set up timer 2 for baudrate
	AUXR &= ~(1 << 3); // as timer
	AUXR |= (1 << 2); // Run T2 at CPU_FREQ
	T2H = reload >> 8;
	T2L = reload;
	IE2 &= ~(1 << 2); // No interrupts from timer 2
	AUXR |= (1 << 4); // Start timer 2

	S2CON |= (1 << 4); // Rx enable
}

void uart1_close()
{
	REN = 0;
	uart1_flush();
	TR1 = 0;
}

void uart2_close()
{
	S2CON &= ~(1 << 4); // Rx disable
	uart2_flush();
	AUXR &= ~(1 << 4); // Stop timer 2
}


uint8_t uart1_available()
{
	return (rx1_head - rx1_tail);
}

uint8_t uart2_available()
{
	return (rx2_head - rx2_tail);
}

uint8_t uart1_read()
{
	ES = 0; // Disable UART1 interrupt
	uint8_t byte = rx1_buf[rx1_tail];
	rx1_tail = (rx1_tail + 1);
	ES = 1; // Enable UART1 interrupt
	return byte;
}

uint8_t uart2_read()
{
	IE2 &= ~(1 << 0); // Disable UART2 interrupt
	uint8_t byte = rx2_buf[rx2_tail];
	rx2_tail = (rx2_tail + 1);
	IE2 |= (1 << 0); // Enable UART2 interrupt
	return byte;
}

void uart1_write(uint8_t byte)
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
		SBUF = byte;
	}
}

void uart2_write(uint8_t byte)
{
	if (tx2_sending)
	{
		// wait for free space in buffer
		while (((tx2_head + 1)) == tx2_tail);

		tx2_buf[tx2_head] = byte;
		tx2_head = (tx2_head + 1);
	}
	else
	{
		tx2_sending = 1;
		S2BUF = byte;
	}
}

void uart1_flush()
{
	while (tx1_sending);
}

void uart2_flush()
{
	while (tx2_sending);
}


INTERRUPT_USING(isr_uart1, IRQ_UART1, 3)
{
	if (RI) // rx interrupt
	{
		RI = 0;		
		if (rx1_head != (rx1_tail - 1))
		{
			rx1_buf[rx1_head] = SBUF;
			rx1_head = (rx1_head + 1);
		}
	}

	if (TI) // tx interrupt
	{
		TI = 0;

		if (tx1_head != tx1_tail)
		{
			tx1_sending = 1;
			SBUF = tx1_buf[tx1_tail];
			tx1_tail = (tx1_tail + 1);
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

		if (rx2_head != (rx2_tail - 1))
		{
			rx2_buf[rx2_head] = S2BUF;
			rx2_head = (rx2_head + 1);
		}
	}

	if (S2CON & (1 << 1)) // tx interrupt
	{
		S2CON &= ~(1 << 1);

		if (tx2_head != tx2_tail)
		{
			tx2_sending = 1;
			S2BUF = tx2_buf[tx2_tail];
			tx2_tail = (tx2_tail + 1);
		}
		else
		{
			tx2_sending = 0;
		}
	}
}

