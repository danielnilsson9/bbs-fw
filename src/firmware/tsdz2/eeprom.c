/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "eeprom.h"
#include "watchdog.h"
#include "tsdz2/cpu.h"
#include "stm8s/stm8s.h"
#include "stm8s/stm8s_flash.h"

#define EEPROM_START_ADDRESS	0x4000

static uint16_t selected_address;

void eeprom_init()
{
	selected_address = EEPROM_START_ADDRESS;
}

bool eeprom_select_page(int page)
{
	if (page >= 0 && page < 2)
	{
		selected_address = EEPROM_START_ADDRESS + (page * 256);
		return true;
	}

	return false;
}

int eeprom_read_byte(int offset)
{
	uint8_t* address = (uint8_t*)(selected_address + offset);
	return *address;
}

bool eeprom_erase_page()
{
	return true; // not needed
}

bool eeprom_write_byte(int offset, uint8_t value)
{
	uint8_t* address = (uint8_t*)(selected_address + offset);

	// disable flash write protection if enabled
	if (!(FLASH->IAPSR & FLASH_IAPSR_DUL))
	{
		FLASH->DUKR = FLASH_RASS_KEY2;
		FLASH->DUKR = FLASH_RASS_KEY1;

		while (!(FLASH->IAPSR & FLASH_IAPSR_DUL));
	}

	watchdog_yeild(); // :TODO: use faster api to write entire page

	*address = value;
	while (!(FLASH->IAPSR & FLASH_IAPSR_EOP));

	return true;
}

bool eeprom_end_write()
{
	// enable write protection
	FLASH->IAPSR &= ~FLASH_IAPSR_DUL;

	return true;
}
