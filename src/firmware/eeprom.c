/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */

#include "eeprom.h"
#include "stc15.h"


#if defined(STC15W4K56)
	#define EEPROM_ADDRESS_OFFSET		0x0000
	#define EEPROM_NUM_SECTORS			4
#elif defined(IAP15W4K61)
	#define EEPROM_ADDRESS_OFFSET		0xEC00
	#define EEPROM_NUM_SECTORS			4
#endif


#define IAP_CMD_IDLE		0
#define IAP_CMD_READ		1
#define IAP_CMD_PROGRAM		2
#define IAP_CMD_ERASE		3


#define IAP_ENABLE			0x82		// Wait time, CPU_FREQ < 20MHz


static __xdata uint16_t selected_sector_offset = 0;


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

bool eeprom_select_page(int page)
{
	if (page >= 0 && page < EEPROM_NUM_SECTORS)
	{
		selected_sector_offset = EEPROM_ADDRESS_OFFSET +  page * 512;
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
