#include "lights.h"
#include "pins.h"
#include "stc15.h"


void lights_init()
{
	SET_PIN_OUTPUT(PIN_LIGHTS_POWER);
	SET_PIN_OUTPUT(PIN_LIGHTS);

	lights_disable();
	lights_set(false);
}

void lights_enable()
{
	SET_PIN_HIGH(PIN_LIGHTS_POWER);
}

void lights_disable()
{
	SET_PIN_LOW(PIN_LIGHTS_POWER);
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
