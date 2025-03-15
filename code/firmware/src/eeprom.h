/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2024.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdbool.h>
#include <stdint.h>

void eeprom_init();
bool eeprom_select_page(int page);

int eeprom_read_byte(int offset);

bool eeprom_erase_page();
bool eeprom_write_byte(int offset, uint8_t value);
bool eeprom_end_write();

#endif
