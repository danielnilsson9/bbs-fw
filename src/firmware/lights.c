#include "lights.h"
#include "pins.h"
#include "stc15.h"


void lights_init()
{
	SET_PIN_OUTPUT(PIN_LIGHTS);
	SET_PIN_LOW(PIN_LIGHTS);
}

void lights_set(bool on)
{
	if (on)
	{
		SET_PIN_HIGH(PIN_LIGHTS);
	}
	else
	{
		SET_PIN_HIGH(PIN_LIGHTS);
	}
}
