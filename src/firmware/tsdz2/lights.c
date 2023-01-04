/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "lights.h"
#include "stm8.h"
#include "pins.h"

static bool lights_enabled;
static bool lights_on;

void lights_init()
{
	SET_PIN_OUTPUT(PIN_LIGHTS);

	lights_enabled = false;
	lights_set(false);
}

void lights_enable()
{
	lights_enabled = true;
	lights_set(lights_on);
}

void lights_disable()
{
	lights_enabled = false;
	lights_set(lights_on);
}

void lights_set(bool on)
{
	lights_on = on;
	if (lights_on && lights_enabled)
	{
		SET_PIN_HIGH(PIN_LIGHTS);
	}
	else
	{
		SET_PIN_LOW(PIN_LIGHTS);
	}
}
