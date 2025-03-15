/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#include "lights.h"
#include "bbsx/pins.h"
#include "bbsx/stc15.h"

void lights_init()
{
    SET_PIN_OUTPUT(PIN_LIGHTS_POWER);
    SET_PIN_OUTPUT(PIN_LIGHTS);

    lights_disable();
    lights_set(false);
}

void lights_enable()
{
    // enable signal level is swapped on BBSHD vs BBS02...
#if defined(BBSHD)
    SET_PIN_HIGH(PIN_LIGHTS_POWER);
#elif defined(BBS02)
    SET_PIN_LOW(PIN_LIGHTS_POWER);
#endif
}

void lights_disable()
{
    // enable signal level is swapped on BBSHD vs BBS02...
#if defined(BBSHD)
    SET_PIN_LOW(PIN_LIGHTS_POWER);
#elif defined(BBS02)
    SET_PIN_HIGH(PIN_LIGHTS_POWER);
#endif
}

void lights_set(bool on)
{
    if (on)
    {
        SET_PIN_LOW(PIN_LIGHTS);
    }
    else
    {
        SET_PIN_HIGH(PIN_LIGHTS);
    }
}
