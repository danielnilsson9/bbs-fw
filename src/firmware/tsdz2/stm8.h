#pragma once

#ifndef _TSDZ2_STM_8_H_
#define _TSDZ2_STM_8_H_

#include <stdint.h>

#define EXPAND(x) x

#define SET_PIN_INPUT_(PORT, PIN) PORT->DDR &= (uint8_t)(~(PIN)); PORT->CR1 &= (uint8_t)(~(PIN))
#define SET_PIN_INPUT(...) EXPAND(SET_PIN_INPUT_(__VA_ARGS__))

#define SET_PIN_INPUT_PULLUP_(PORT, PIN) PORT->DDR &= (uint8_t)(~(PIN)); PORT->CR1 |= (uint8_t)PIN
#define SET_PIN_INPUT_PULLUP(...) EXPAND(SET_PIN_INPUT_PULLUP_(__VA_ARGS__))

#define SET_PIN_OUTPUT_(PORT, PIN)  PORT->DDR |= (uint8_t)PIN; PORT->CR1 |= (uint8_t)PIN; PORT->CR2 |= (uint8_t)(PIN)
#define SET_PIN_OUTPUT(...) EXPAND(SET_PIN_OUTPUT_(__VA_ARGS__))

#define SET_PIN_OUTPUT_OPEN_DRAIN_(PORT, PIN)  PORT->DDR |= (uint8_t)PIN; PORT->CR1 &= (uint8_t)(~(PIN)); PORT->CR2 |= (uint8_t)(PIN)
#define SET_PIN_OUTPUT_OPEN_DRAIN(...) EXPAND(SET_PIN_OUTPUT_OPEN_DRAIN_(__VA_ARGS__))


#define GET_PIN_INPUT_STATE_(PORT, PIN) ((PORT->IDR & (uint8_t)PIN) != 0)
#define GET_PIN_INPUT_STATE(...) EXPAND(GET_PIN_INPUT_STATE_(__VA_ARGS__))

#define SET_PIN_HIGH_(PORT, PIN) PORT->ODR |= (uint8_t)PIN
#define SET_PIN_HIGH(...) EXPAND(SET_PIN_HIGH_(__VA_ARGS__))

#define SET_PIN_LOW_(PORT, PIN) PORT->ODR &= (uint8_t)(~PIN)
#define SET_PIN_LOW(...) EXPAND(SET_PIN_LOW_(__VA_ARGS__))

#define TOGGLE_PIN_(PORT, PIN) PORT->ODR ^= (PIN)
#define TOGGLE_PIN(...) EXPAND(TOGGLE_PIN_(__VA_ARGS__))


#define GET_PIN_(PORT, PIN) PIN
#define GET_PIN(...) EXPAND(GET_PIN_(__VA_ARGS__))

#define GET_PORT_(PORT, PIN) PORT
#define GET_PORT(...) EXPAND(GET_PORT_(__VA_ARGS__))


#endif
