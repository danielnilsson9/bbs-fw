/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _STC_15_H_
#define _STC_15_H_

#include "intellisense.h"

#if !defined (SDCC) && !defined (__SDCC)

#define __sfr		unsigned char
#define __sbit		bool
#define __at(X)		

#define __xdata
#define __data		

#endif

#include <8051.h>
#include <stc12.h>


// Peripheral function switch
SFR(P_SW2, 0xBA);

SFR(PCON2, 0x97);

SFR(T2H, 0xD6);
SFR(T2L, 0xD7);

SBIT(P5_4, 0xC8, 4);
SBIT(P5_5, 0xC8, 5);


#define IS_BIT_SET(REG, BIT_NUM) ((REG >> BIT_NUM) & 1)

#define SET_BIT(REG, BIT_NUM) (REG |= (1 << BIT_NUM))
#define CLEAR_BIT(REG, BIT_NUM) (REG &= ~(1 << BIT_NUM))
#define TOGGLE_BIT(REG, BIT_NUM) (REG ^= (1 << BIT_NUM))

#define EXPAND(x) x

#define SET_PIN_INPUT_(PORT, PIN) CLEAR_BIT(P##PORT##M0, PIN); SET_BIT(P##PORT##M1, PIN)
#define SET_PIN_INPUT(...) EXPAND(SET_PIN_INPUT_(__VA_ARGS__))

#define SET_PIN_QUASI_(PORT, PIN) CLEAR_BIT(P##PORT##M0, PIN); CLEAR_BIT(P##PORT##M1, PIN)
#define SET_PIN_QUASI(...) EXPAND(SET_PIN_QUASI_(__VA_ARGS__))

#define SET_PIN_OUTPUT_(PORT, PIN) SET_BIT(P##PORT##M0, PIN); CLEAR_BIT(P##PORT##M1, PIN)
#define SET_PIN_OUTPUT(...) EXPAND(SET_PIN_OUTPUT_(__VA_ARGS__))


#define GET_PIN_STATE_(PORT, PIN) P##PORT##_##PIN
#define GET_PIN_STATE(...) EXPAND(GET_PIN_STATE_(__VA_ARGS__))

#define SET_PIN_HIGH_(PORT, PIN) P##PORT##_##PIN = 1
#define SET_PIN_HIGH(...) EXPAND(SET_PIN_HIGH_(__VA_ARGS__))

#define SET_PIN_LOW_(PORT, PIN) P##PORT##_##PIN = 0
#define SET_PIN_LOW(...) EXPAND(SET_PIN_LOW_(__VA_ARGS__))


#define GET_PIN_NUM_(PORT, PIN) PIN
#define GET_PIN_NUM(...) EXPAND(GET_PIN_NUM_(__VA_ARGS__))

#define GET_PORT_NUM_(PORT, PIN) PORT
#define GET_PORT_NUM(...) EXPAND(GET_PORT_NUM_(__VA_ARGS__))

#endif
