/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "eeprom.h"
#include "bbsx/stc15.h"


#define EEPROM_NUM_SECTORS			4

 // STC chips has a special area in flash for eeprom.
#define EEPROM_STC_ADDRESS_OFFSET	0x0000

// IAP chips have no special area, same area as program
// memory and address space is the same. We define the last
// four sectors for eeprom usage ourself.
#define EEPROM_IAP_ADDRESS_OFFSET	0xEC00

#define IAP_CMD_IDLE		0
#define IAP_CMD_READ		1
#define IAP_CMD_PROGRAM		2
#define IAP_CMD_ERASE		3

#define IAP_ENABLE			0x82		// Wait time, CPU_FREQ < 20MHz


static uint16_t address_offset = 0x0000;
static uint16_t selected_sector_offset = 0;


static void eeprom_begin(uint8_t cmd, int offset)
{
	IAP_CONTR = IAP_ENABLE;
	IAP_CMD = cmd;

	uint16_t addr = selected_sector_offset + offset;
	IAP_ADDRH = addr >> 8;
	IAP_ADDRL = addr;
}

static bool eeprom_trigger()
{
	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	NOP();

	return !IS_BIT_SET(IAP_CONTR, 4);
}

static void eeprom_end()
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0xff;
	IAP_ADDRL = 0xff;
}

void eeprom_init()
{
	// Detect if we are running on IAP or STC model dependeing on if
	// we can read from IAP address offset which is outside eeprom
	// address space on STC model.

	address_offset = EEPROM_IAP_ADDRESS_OFFSET;
	eeprom_select_page(0);
	if (eeprom_read_byte(0) == -1)
	{
		address_offset = EEPROM_STC_ADDRESS_OFFSET;
	}
}

bool eeprom_select_page(int page)
{
	if (page >= 0 && page < EEPROM_NUM_SECTORS)
	{
		selected_sector_offset = address_offset + page * 512;
		return true;
	}

	return false;
}

bool eeprom_erase_page()
{
	bool res;

	eeprom_begin(IAP_CMD_ERASE, 0);
	res = eeprom_trigger();
	eeprom_end();

	return res;
}

int eeprom_read_byte(int offset)
{
	int res = -1;

	eeprom_begin(IAP_CMD_READ, offset);

	if (eeprom_trigger())
	{
		res = IAP_DATA;
	}

	eeprom_end();

	return res;
}

bool eeprom_write_byte(int offset, uint8_t value)
{
	bool res;

	eeprom_begin(IAP_CMD_PROGRAM, offset);
	IAP_DATA = value;
	res = eeprom_trigger();
	eeprom_end();

	return res;
}

bool eeprom_end_write()
{
	return true;
}
