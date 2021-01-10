/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _STC_15_H_
#define _STC_15_H_

// NOTE:
// The defines below is here to keep IntelliSense 
// in Visual Studio happy and not throw incorrect errors.

#if !defined (SDCC) && !defined (__SDCC)

#define __sfr		unsigned char
#define __sbit		bool
#define __at(X)		

#define __xdata		

#define INTERRUPT(name, vector)					void name()
#define INTERRUPT_USING(name, vector,regnum)	void name()

#endif

#endif

#include <8051.h>
#include <stc12.h>

// Peripheral function switch
SFR(P_SW2, 0xBA);

SFR(T2H, 0xD6);
SFR(T2L, 0xD7);
