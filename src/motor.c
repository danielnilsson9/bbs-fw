/*
 * bbshd-fw
 *
 * Copyright (C) Daniel Nilsson, 2021.
 *
 * Released under the GPL License, Version 3
 */
#include "motor.h"
#include "system.h"
#include "uart.h"

#define OPCODE_LVC				0x60
#define OPCODE_MAX_CURRENT		0x61
#define OPCODE_TARGET_CURRENT	0x63
#define OPCODE_UNKNOWN8			0x64
#define OPCODE_HELLO			0x67
#define OPCODE_UNKNOWN1			0x68
#define OPCODE_UNKNOWN2			0x69
#define OPCODE_UNKNOWN3			0x6A
#define OPCODE_UNKNOWN4			0x6B
#define OPCODE_UNKNOWN5			0x6C
#define OPCODE_UNKNOWN5			0x6C
#define OPCODE_UNKNOWN6			0x6D
#define OPCODE_UNKNOWN7			0x6E

#define OPCODE_READ_STATUS		0x40
#define OPCODE_READ_CURRENT		0x41
#define OPCODE_READ_VOLTAGE		0x42

#define READ_TIMEOUT			100


static uint8_t is_connected = 0;
static uint8_t msgbuf[8];

static uint32_t next_read_status_ms = 0;


uint8_t compute_checksum(uint8_t* msg, uint8_t len)
{
	uint8_t checksum = 0;
	for (int i = 0; i < len; ++i)
	{
		checksum += *(msg + i);
	}

	return checksum;
}

void send_request(uint8_t opcode, uint16_t data)
{
	uint8_t idx = 0;

	msgbuf[idx++] = 0xaa; // start of message
	msgbuf[idx++] = opcode;

	if (opcode == OPCODE_LVC)
	{
		msgbuf[idx++] = data >> 8;
		msgbuf[idx++] = data;
	}
	else if (opcode != OPCODE_READ_STATUS && opcode != OPCODE_READ_CURRENT && opcode != OPCODE_READ_VOLTAGE)
	{
		msgbuf[idx++] = data;
	}

	uint8_t checksum = compute_checksum(msgbuf + 1, idx - 1);
	msgbuf[idx++] = checksum;

	// empty rx buffer
	while (uart2_available()) uart2_read();

	for (uint8_t i = 0; i < idx; ++i)
	{
		uart2_write(msgbuf[i]);
	}

	uart2_flush();
}

int read_response(uint8_t opcode, uint16_t* out_data)
{
	uint8_t read = 0;
	uint32_t start = system_ms();

	uint8_t len = (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE) ? 5 : 4;

	uint8_t i = 0;
	while (i < len && system_ms() < start + READ_TIMEOUT)
	{
		if (uart2_available())
		{
			msgbuf[i++] = uart2_read();
		}
	}

	if (i > 1 && msgbuf[1] == opcode)
	{
		uint8_t checksum = compute_checksum(&msgbuf[1], (uint8_t)(i - 2));
		if (checksum == msgbuf[i - 1])
		{
			if (out_data != 0)
			{
				if (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE)
				{
					*out_data = msgbuf[2] << 8 | msgbuf[3];
				}
				else
				{
					*out_data = msgbuf[2];
				}
			}
			
			return 1;
		}

		return 0; // failed to verify message
	}
	
	// read failure
	return 0;
}


int motor_connect()
{
	for (int i = 0; i < 10; ++i)
	{
		send_request(OPCODE_HELLO, 0x00);
	
		if (read_response(OPCODE_HELLO, 0))
		{
			system_delay_ms(10);
			return 1;
		}
		else
		{
			system_delay_ms(1000);
		}		
	}

	return 0;
}

int motor_configure(uint16_t max_current_mA, uint8_t lvc_V)
{
	send_request(OPCODE_UNKNOWN1, 0x5a);
	if (!read_response(OPCODE_UNKNOWN1, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN2, 0x11);
	if (!read_response(OPCODE_UNKNOWN2, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN3, 0x78);
	if (!read_response(OPCODE_UNKNOWN3, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN4, 0x64);
	if (!read_response(OPCODE_UNKNOWN4, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN5, 0x50);
	if (!read_response(OPCODE_UNKNOWN5, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN6, 0x46);
	if (!read_response(OPCODE_UNKNOWN6, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN7, 0x0c);
	if (!read_response(OPCODE_UNKNOWN7, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_LVC, lvc_V * 14);
	if (!read_response(OPCODE_LVC, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_MAX_CURRENT, (uint16_t)((max_current_mA * 69UL) / 10000UL));
	if (!read_response(OPCODE_MAX_CURRENT, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	send_request(OPCODE_UNKNOWN8, 0x1c);
	if (!read_response(OPCODE_UNKNOWN8, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	// investigate if this is needed, seems strange
	send_request(OPCODE_TARGET_CURRENT, 0x0a);
	if (!read_response(OPCODE_TARGET_CURRENT, 0))
	{
		return 0;
	}

	system_delay_ms(10);

	return 1;
}

void motor_init(uint16_t max_current_mA, uint8_t lvc_V)
{
	P2M1 &= ~(1 << 0);
	P2M0 |= (1 << 0);

	P2M1 &= ~(1 << 1);
	P2M0 |= (1 << 1);
	
	P4M1 &= ~(1 << 4);
	P4M0 |= (1 << 4);

	P2_0 = 0;
	P4_4 = 0;
	P2_1 = 1;

	uart2_open(4800);

	// Extra wait for other MCU to power on
	system_delay_ms(100);


	if (motor_connect() && motor_configure(max_current_mA, lvc_V))
	{
		is_connected = 1;

		uart1_write('M');
		uart1_write('C');
		uart1_write('\n');
	}
	else
	{
		uart1_write('M');
		uart1_write('E');
		uart1_write('\n');
	}
}

void motor_enable()
{
	P2_0 = 1;
	system_delay_ms(10);
}

void motor_disable()
{
	P2_0 = 0;
}

void motor_set_target_current(uint16_t target_mA)
{
	send_request(OPCODE_TARGET_CURRENT, (uint16_t)((target_mA * 69UL) / 10000UL));
	read_response(OPCODE_TARGET_CURRENT, 0);
	system_delay_ms(2);
}


void read_status()
{
	uint16_t status = 0;

	send_request(OPCODE_READ_STATUS, 0);
	read_response(OPCODE_READ_STATUS, &status);

	uart1_write(status >> 8);
	uart1_write(status);

	system_delay_ms(2);

	send_request(OPCODE_READ_CURRENT, 0);
	read_response(OPCODE_READ_CURRENT, 0);

	system_delay_ms(2);

	send_request(OPCODE_READ_VOLTAGE, 0);
	read_response(OPCODE_READ_VOLTAGE, 0);

	system_delay_ms(2);
}

void motor_process()
{
	if (is_connected && system_ms() > next_read_status_ms)
	{
		next_read_status_ms = system_ms() + 150;
		read_status();
	}
}

