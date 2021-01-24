/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>
#include <stdbool.h>

bool eeprom_select_page(int page);

bool eeprom_erase_page();
int eeprom_read_byte(int offset);
bool eeprom_write_byte(int offset, uint8_t value);


#endif
