/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef  _INTERRUPT_H_
#define _INTERRUPT_H_

// Interrupt rouines declarations required to be included from main.c
#if defined(BBSHD) || defined(BBS02)
#include "bbsx/interrupt.h"
#elif defined(TSDZ2)
#include "tsdz2/interrupt.h"
#endif

#endif
