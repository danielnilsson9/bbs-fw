/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "motor.h"
#include "sensors.h"
#include "system.h"
#include "eventlog.h"
#include "bbsx/uart_motor.h"
#include "bbsx/pins.h"

#include <stdbool.h>

#define OPCODE_LVC				0x60
#define OPCODE_MAX_CURRENT		0x61
#define OPCODE_TARGET_SPEED		0x63
#define OPCODE_TARGET_CURRENT	0x64
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

#if defined(BBSHD)
	#define ADC_STEPS_PER_AMP_X10		69
	#define ADC_STEPS_PER_VOLT_X100		1490 // 1460 in orginal firmware
#elif defined(BBS02)
	#define ADC_STEPS_PER_AMP_X10		56
	#define ADC_STEPS_PER_VOLT_X100		1510
#endif

#define SPEED_STEPS					250

// async om state machine
#define COM_STATE_IDLE				0x01
#define COM_STATE_WAIT_RESPONSE		0x02
#define COM_STATE_SET_CURRENT		0x03
#define COM_STATE_SET_SPEED			0x04
#define COM_STATE_READ_STATUS		0x05
#define COM_STATE_READ_CURRENT		0x06
#define COM_STATE_READ_VOLTAGE		0x07

#define MSGBUF_SIZE					8

static uint8_t is_connected;
static uint8_t msgbuf[MSGBUF_SIZE];

static bool target_speed_changed;
static uint8_t target_speed;

static bool target_current_changed;
static uint8_t target_current;

static uint16_t adc_steps_per_volt_x100;
static uint16_t lvc_volt_x10;

static uint16_t status_flags;
static uint16_t battery_volt_x10;
static uint16_t battery_adc_steps;
static uint16_t battery_amp_x10;

// state machine state
static uint8_t com_state;
static uint8_t last_sent_opcode;
static uint32_t last_request_write_ms;
static uint32_t last_status_read_ms;
static uint8_t next_status_read_opcode;


static uint8_t compute_checksum(uint8_t* msg, uint8_t len);
static void send_request(uint8_t opcode, uint16_t data);
static void send_request_async(uint8_t opcode, uint16_t data);

static int read_response(uint8_t opcode, uint16_t* out_data);
static int try_read_response(uint8_t opcode, uint16_t* out_data);
static int connect();
static int configure(uint16_t max_current_mA, uint8_t lvc_V);

static void process_com_state_machine();


void motor_pre_init()
{
	SET_PIN_OUTPUT(PIN_MOTOR_POWER_ENABLE);
	SET_PIN_OUTPUT(PIN_MOTOR_CONTROL_ENABLE);
	SET_PIN_OUTPUT(PIN_MOTOR_EXTRA);

	SET_PIN_LOW(PIN_MOTOR_POWER_ENABLE);
	SET_PIN_HIGH(PIN_MOTOR_CONTROL_ENABLE);
	SET_PIN_HIGH(PIN_MOTOR_EXTRA);
}

void motor_init(uint16_t max_current_mA, uint8_t lvc_V, int16_t adc_calib_volt_steps_x100)
{
	motor_pre_init();

	is_connected = 0;
	target_speed_changed = false;
	target_speed = 0;
	target_current_changed = false;
	target_current = 0;
	status_flags = 0;
	adc_steps_per_volt_x100 = ADC_STEPS_PER_VOLT_X100 + adc_calib_volt_steps_x100;
	lvc_volt_x10 = (uint16_t)lvc_V * 10;
	battery_volt_x10 = 0;
	battery_adc_steps = 0;
	battery_amp_x10 = 0;

	com_state = COM_STATE_IDLE;
	last_sent_opcode = 0;
	last_request_write_ms = 0;
	last_status_read_ms = 0;
	next_status_read_opcode = OPCODE_READ_STATUS;

	uart_motor_open(4800);

	// Give other MCU time to power on
	while (system_ms() < 100);

	if (connect() && configure(max_current_mA, lvc_V))
	{
		is_connected = 1;

		eventlog_write(EVT_MSG_MOTOR_INIT_OK);

		motor_set_target_speed(0);
		motor_set_target_current(0);
		target_current_changed = true;
		target_speed_changed = true;
	}
	else
	{
		eventlog_write(EVT_ERROR_INIT_MOTOR);
	}
}

void motor_process()
{
	if (!is_connected)
	{
		return;
	}

	process_com_state_machine();
}

void motor_enable()
{
	SET_PIN_HIGH(PIN_MOTOR_POWER_ENABLE);
}

void motor_disable()
{
	if (!brake_is_activated())
	{
		// Brake signal is also connected to motor control MCU.
		// If we disable motor power here during braking it causes
		// a small issue where change in target current is not accepted
		// while in disabled state. This will result in a short power spike
		// when brake eventually released.

		SET_PIN_LOW(PIN_MOTOR_POWER_ENABLE);
	}	
}

uint16_t motor_status()
{
	return status_flags;
}

uint8_t motor_get_target_speed()
{
	return target_speed;
}

uint8_t motor_get_target_current()
{
	return target_current;
}


void motor_set_target_speed(uint8_t percent)
{
	if (percent > 100)
	{
		percent = 100;
	}

	if (target_speed != percent)
	{
		target_speed = percent;
		target_speed_changed = true;
	}
}

void motor_set_target_current(uint8_t percent)
{
	if (percent > 100)
	{
		percent = 100;
	}

	if (target_current != percent)
	{
		target_current = percent;
		target_current_changed = true;
	}
}

int16_t motor_calibrate_battery_voltage(uint16_t actual_voltage_x100)
{
	int16_t diff = 0;
	if (actual_voltage_x100 != 0)
	{
		uint16_t calibrated_adc_steps_volt_x100 = (uint16_t)(((uint32_t)battery_adc_steps * 10000u) / actual_voltage_x100);
		diff = calibrated_adc_steps_volt_x100 - ADC_STEPS_PER_VOLT_X100;

		adc_steps_per_volt_x100 = calibrated_adc_steps_volt_x100;
	}
	else
	{
		// reset calibration if 0 is received
		adc_steps_per_volt_x100 = ADC_STEPS_PER_VOLT_X100;
		diff = 0;
	}

	eventlog_write_data(EVT_DATA_CALIBRATE_VOLTAGE, adc_steps_per_volt_x100);

	return diff;
}


uint16_t motor_get_battery_lvc_x10()
{
	return lvc_volt_x10;
}

uint16_t motor_get_battery_current_x10()
{
	return battery_amp_x10;
}

uint16_t motor_get_battery_voltage_x10()
{
	return battery_volt_x10;
}

static uint8_t compute_checksum(uint8_t* msg, uint8_t len)
{
	uint8_t checksum = 0;
	for (int i = 0; i < len; ++i)
	{
		checksum += *(msg + i);
	}

	return checksum;
}

static void send_request(uint8_t opcode, uint16_t data)
{
	// empty rx buffer
	while (uart_motor_available()) uart_motor_read();

	send_request_async(opcode, data);

	uart_motor_flush();
}

static void send_request_async(uint8_t opcode, uint16_t data)
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

	for (uint8_t i = 0; i < idx; ++i)
	{
		uart_motor_write(msgbuf[i]);
	}
}

static int read_response(uint8_t opcode, uint16_t* out_data)
{
	uint32_t end = system_ms() + READ_TIMEOUT;

	uint8_t len = (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE) ? 5 : 4;

	uint8_t i = 0;
	while (i < len && system_ms() < end)
	{
		if (uart_motor_available())
		{
			msgbuf[i++] = uart_motor_read();
		}
	}

	if (i == len && msgbuf[1] == opcode)
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

static int try_read_response(uint8_t opcode, uint16_t* out_data)
{
	uint8_t len = (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE) ? 5 : 4;

	uint8_t i = 0;
	while (uart_motor_available() && i < MSGBUF_SIZE)
	{
		msgbuf[i++] = uart_motor_read();
	}

	// clear anything that could be left in rxbuffer in case of error.
	while (uart_motor_available()) uart_motor_read();

	if (i < len)
	{
		// failed to read entire response
		return 0;
	}

	if (i == len && msgbuf[1] == opcode)
	{
		uint8_t checksum = compute_checksum(&msgbuf[1], (uint8_t)(i - 2));
		if (checksum == msgbuf[i - 1])
		{
			if (out_data != 0)
			{
				if (opcode == OPCODE_LVC || opcode == OPCODE_READ_STATUS || opcode == OPCODE_READ_VOLTAGE)
				{
					*out_data = ((uint16_t)msgbuf[2] << 8) | msgbuf[3];
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


static int connect()
{
	for (int i = 0; i < 10; ++i)
	{
		send_request(OPCODE_HELLO, 0x00);

		if (read_response(OPCODE_HELLO, 0))
		{
			system_delay_ms(4);
			return 1;
		}
		else
		{
			system_delay_ms(1000);
		}
	}

	return 0;
}

static int configure(uint16_t max_current_mA, uint8_t lvc_V)
{
	uint16_t tmp = 0;

	// This initialization is done exactly as in orginal firmware for BBSHD/BBS02.
	// The meaning of most parameters is unknown.

#if defined (BBSHD)
	send_request(OPCODE_UNKNOWN1, 0x5a);
#elif defined (BBS02)
	send_request(OPCODE_UNKNOWN1, 0x5f);
#else
	return 0;
#endif

	if (!read_response(OPCODE_UNKNOWN1, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN2, 0x11);
	if (!read_response(OPCODE_UNKNOWN2, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN3, 0x78);
	if (!read_response(OPCODE_UNKNOWN3, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN4, 0x64);
	if (!read_response(OPCODE_UNKNOWN4, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN5, 0x50);
	if (!read_response(OPCODE_UNKNOWN5, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN6, 0x46);
	if (!read_response(OPCODE_UNKNOWN6, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_UNKNOWN7, 0x0c);
	if (!read_response(OPCODE_UNKNOWN7, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	send_request(OPCODE_LVC, (uint32_t)(((uint32_t)lvc_V * adc_steps_per_volt_x100) / 100u));
	if (!read_response(OPCODE_LVC, 0))
	{
		return 0;
	}

	system_delay_ms(4);

	tmp = (uint16_t)((max_current_mA * (uint32_t)ADC_STEPS_PER_AMP_X10) / 10000UL);
	if (tmp > 255)
	{
		tmp = 255;
	}
	eventlog_write_data(EVT_DATA_MAX_CURRENT_ADC_REQUEST, tmp);

	send_request(OPCODE_MAX_CURRENT, tmp);
	if (!read_response(OPCODE_MAX_CURRENT, &tmp))
	{
		return 0;
	}
	else
	{
		eventlog_write_data(EVT_DATA_MAX_CURRENT_ADC_RESPONSE, tmp);
	}

	system_delay_ms(4);

	return 1;
}


static void process_com_state_machine_idle()
{
	// Async state machine loop for serial communication with motor control MCU.
	//
	// Handles:
	// * Set target current
	// * Set target speed
	// * Read motor status
	// * Read motor current
	// * Read battery voltage
	//
	// Set target speed/current are prioritzed over status reading (shorter check interval).

	uint32_t now = system_ms();

	// make sure requests have some space between them
	if (now - last_request_write_ms < 32)
	{
		return;
	}

	if (target_current_changed)
	{
		send_request_async(OPCODE_TARGET_CURRENT, target_current);
		last_sent_opcode = OPCODE_TARGET_CURRENT;
		last_request_write_ms = now;
		com_state = COM_STATE_WAIT_RESPONSE;
		target_current_changed = false;
		return;
	}

	if (target_speed_changed)
	{
		send_request_async(OPCODE_TARGET_SPEED, (uint8_t)(((uint16_t)SPEED_STEPS * target_speed) / 100));
		last_sent_opcode = OPCODE_TARGET_SPEED;
		last_request_write_ms = now;
		com_state = COM_STATE_WAIT_RESPONSE;
		target_speed_changed = false;
		return;
	}

	if ((now - last_status_read_ms) > 200)
	{
		send_request_async(next_status_read_opcode, 0);
		last_sent_opcode = next_status_read_opcode;
		last_request_write_ms = now;
		com_state = COM_STATE_WAIT_RESPONSE;
		if (next_status_read_opcode == OPCODE_READ_STATUS)
		{
			last_status_read_ms = now;
		}
		return;
	}
}

static void process_com_state_machine_wait_response()
{
	uint8_t response_length = 0;

	switch (last_sent_opcode)
	{
	case OPCODE_TARGET_CURRENT:
	case OPCODE_TARGET_SPEED:
	case OPCODE_READ_CURRENT:
		response_length = 4;
		break;
	case OPCODE_READ_VOLTAGE:
	case OPCODE_READ_STATUS:
		response_length = 5;
		break;
	}

	if (uart_motor_available() >= response_length || (system_ms() - last_request_write_ms) > 32)
	{
		switch (last_sent_opcode)
		{
		case OPCODE_TARGET_CURRENT:
			com_state = COM_STATE_SET_CURRENT;
			break;
		case OPCODE_TARGET_SPEED:
			com_state = COM_STATE_SET_SPEED;
			break;
		case OPCODE_READ_CURRENT:
			com_state = COM_STATE_READ_CURRENT;
			break;
		case OPCODE_READ_VOLTAGE:
			com_state = COM_STATE_READ_VOLTAGE;
			break;
		case OPCODE_READ_STATUS:
			com_state = COM_STATE_READ_STATUS;
			break;
		default:
			com_state = COM_STATE_IDLE;
			break;
		}
	}
}

static void process_com_state_machine()
{
	uint16_t data;
	switch (com_state)
	{
	case COM_STATE_IDLE:
		process_com_state_machine_idle();
		break;

	case COM_STATE_WAIT_RESPONSE:
		process_com_state_machine_wait_response();
		break;

	case COM_STATE_SET_CURRENT:
		if (try_read_response(OPCODE_TARGET_CURRENT, &data))
		{
			eventlog_write_data(EVT_DATA_TARGET_CURRENT, data);
		}
		else
		{
			eventlog_write(EVT_ERROR_CHANGE_TARGET_CURRENT);
		}

		com_state = COM_STATE_IDLE;
		break;

	case COM_STATE_SET_SPEED:
		if (try_read_response(OPCODE_TARGET_SPEED, &data))
		{
			eventlog_write_data(EVT_DATA_TARGET_SPEED, (uint8_t)((data * 100) / SPEED_STEPS));
		}
		else
		{
			eventlog_write(EVT_ERROR_CHANGE_TARGET_SPEED);
		}

		com_state = COM_STATE_IDLE;
		break;

	case COM_STATE_READ_STATUS:
		if (try_read_response(OPCODE_READ_STATUS, &data))
		{
			if (data != status_flags)
			{
				status_flags = data;
				eventlog_write_data(EVT_DATA_MOTOR_STATUS, status_flags);
			}
		}
		else
		{
			eventlog_write(EVT_ERROR_READ_MOTOR_STATUS);
		}

		next_status_read_opcode = OPCODE_READ_CURRENT;
		com_state = COM_STATE_IDLE;
		break;

	case COM_STATE_READ_CURRENT:
		if (try_read_response(OPCODE_READ_CURRENT, &data))
		{
			battery_amp_x10 = (data * 100) / ADC_STEPS_PER_AMP_X10;
		}
		else
		{
			eventlog_write(EVT_ERROR_READ_MOTOR_CURRENT);
		}

		next_status_read_opcode = OPCODE_READ_VOLTAGE;
		com_state = COM_STATE_IDLE;
		break;

	case COM_STATE_READ_VOLTAGE:
		if (try_read_response(OPCODE_READ_VOLTAGE, &data))
		{
			battery_adc_steps = data;
			battery_volt_x10 = (uint16_t)(((uint32_t)battery_adc_steps * 1000) / adc_steps_per_volt_x100);
		}
		else
		{
			eventlog_write(EVT_ERROR_READ_MOTOR_VOLTAGE);
		}

		next_status_read_opcode = OPCODE_READ_STATUS;
		com_state = COM_STATE_IDLE;
		break;

	}
}
