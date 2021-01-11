/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _PAS_H_
#define _PAS_H_

#include "stc15.h"
#include "interrupt.h"

#include <stdint.h>
#include <stdbool.h>

void pas_init();

uint8_t pas_get_rpm();
bool pas_is_pedaling_forwards();
bool pas_is_pedaling_backwards();

INTERRUPT(isr_timer4, IRQ_TIMER4);

#endif
