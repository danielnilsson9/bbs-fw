/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2020.
 *
 * Released under the GPL License, Version 3
 */

#include "stc15.h"


void delay_ms(unsigned char ms)
{
	unsigned char i, j;
	do {
		i = 4;
		j = 200;
		do
		{
			while (--j);
		} while (--i);
	} while (--ms);
}

void main(void)
{
	while (1)
	{
		P0_1 = 0;
		delay_ms(200);
		P0_1 = 1;
		delay_ms(200);

		WDT_CONTR |= 1 << 4;
	}
}