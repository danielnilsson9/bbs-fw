#include "timers.h"
#include "bbsx/timers.h"
#include "bbsx/interrupt.h"
#include "bbsx/cpu.h"

#include <stdbool.h>

#define TIMER0_RELOAD	((65535 - CPU_FREQ / 10000) + 1)


extern void system_timer0_isr();
extern void sensors_timer0_isr();

static bool timer0_system_ready;
static bool timer0_sensors_ready;

static void timer0_init()
{
	if (timer0_system_ready || timer0_sensors_ready)
	{
		// already initialized
		return;
	}

	EA = 0; // disable interrupts

	TMOD = (TMOD & 0xf0) | 0x00; // Timer 0: 16-bit with autoreload
	AUXR |= 0x80; // Run timer 0 at CPU_FREQ

	TH0 = TIMER0_RELOAD >> 8;
	TL0 = TIMER0_RELOAD;

	EA = 1; // enable interrupts
	ET0 = 1; // enable timer0 interrupts
	TR0 = 1; // start timer 0
}


void timers_init()
{
	timer0_system_ready = false;
	timer0_sensors_ready = false;
}

void timer0_init_system()
{
	timer0_init();
	timer0_system_ready = true;
}

void timer0_init_sensors()
{
	timer0_init();
	timer0_sensors_ready = true;
}

void timer1_init_uart1(uint32_t baudrate)
{
	unsigned short reload = 65535 - CPU_FREQ / 4 / baudrate + 1;

	// Set up timer 1 for baudrate
	TMOD = (TMOD & 0x0f) | 0x00; // Run T1 in mode 0 (16-bit reload)
	AUXR |= 0x40; // Run T1 at CPU_FREQ
	TL1 = reload; // Set the reload value for given baudrate.
	TH1 = reload >> 8;
	ET1 = 0; // No interrupts from timer 1.
	TR1 = 1; // Start timer 1
}

void timer2_init_uart2(uint32_t baudrate)
{
	unsigned short reload = 65535 - CPU_FREQ / 4 / baudrate + 1;

	// Set up timer 2 for baudrate
	AUXR &= ~(1 << 3); // as timer
	AUXR |= (1 << 2); // Run T2 at CPU_FREQ
	T2H = reload >> 8;
	T2L = reload;
	IE2 &= ~(1 << 2); // No interrupts from timer 2
	AUXR |= (1 << 4); // Start timer 2
}


// timer0 is shared between system ms counter and sensors check
INTERRUPT_USING(isr_timer0, IRQ_TIMER0, 1)
{
	if (timer0_system_ready)
	{
		system_timer0_isr();
	}
	
	if (timer0_sensors_ready)
	{
		sensors_timer0_isr();
	}
}
