/*
 * TongSheng TSDZ2 motor controller firmware/
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 * 
 * - Original motor control code from TongSheng TSDZ2 motor controller firmware.
 * - 9bit motor pwm from fork by Frans-Willem.
 * - Cleaned up and integrated into bbs-fw by Daniel Nilsson.
 */
#include <stdbool.h>
#include "motor.h"
#include "system.h"
#include "uart.h"
#include "eventlog.h"
#include "util.h"
#include "tsdz2/cpu.h"
#include "tsdz2/timers.h"
#include "tsdz2/pins.h"
#include "tsdz2/stm8.h"
#include "tsdz2/stm8s/stm8s.h"
#include "tsdz2/stm8s/stm8s_tim1.h"
#include "tsdz2/stm8s/stm8s_itc.h"
#include "tsdz2/stm8s/stm8s_adc1.h"
#include "tsdz2/stm8s/stm8s_flash.h"


// Motor
// ---------------------------------------------------------------------------------

// hard current limits
#define MAX_BATTERY_CURRENT_AMPS_X10		200
#define MAX_MOTOR_PHASE_CURRENT_AMPS_X10	300


// Maximum current ramp
// ----------------------------------------------
// Every second has 15625 pwm cycles interrupts,
// one ADC battery current step  -> 0.156 amps:
//
// A / 0.156 = X (we need to do X steps ramp up per second)
// Therefore :
// 15625 / (A / 0.156) => (15625 * 0.156) / A
//
// 20A/s: (15625 * 0.156) / 20 = 135
#define CURRENT_RAMP_UP_INVERSE_STEP			135

// Choose PWM ramp up/down step (higher value will make the motor acceleration slower)
//
// For a 24V battery, 25 for ramp up seems ok. For an higher voltage battery, this values should be higher
#define PWM_DUTY_CYCLE_RAMP_UP_INVERSE_STEP		24
#define PWM_DUTY_CYCLE_RAMP_DOWN_INVERSE_STEP	28

// This value should be near 0.
// You can try to tune with the whell on the air, full throttle and look at batttery current: adjust for lower battery current
#define MOTOR_ROTOR_OFFSET_ANGLE				9

// This value is ERPS speed after which a transition happens from sinewave no interpolation to have
// interpolation 60 degrees and must be found experimentally
#define MOTOR_ROTOR_ERPS_START_INTERPOLATION_60_DEGREES 10

#define PWM_CYCLES_COUNTER_MAX					3125U	// 5 erps minimum speed; 1/5 = 200ms; 200ms/64us = 3125
#define PWM_CYCLES_SECOND						15625U	// 1 / 64us (PWM period)
#define PWM_DUTY_CYCLE_MAX						254
#define PWM_DUTY_CYCLE_MIN						20

#define MOTOR_ROTOR_ANGLE_90					(63  + MOTOR_ROTOR_OFFSET_ANGLE)
#define MOTOR_ROTOR_ANGLE_150					(106 + MOTOR_ROTOR_OFFSET_ANGLE)
#define MOTOR_ROTOR_ANGLE_210					(148 + MOTOR_ROTOR_OFFSET_ANGLE)
#define MOTOR_ROTOR_ANGLE_270					(191 + MOTOR_ROTOR_OFFSET_ANGLE)
#define MOTOR_ROTOR_ANGLE_330					(233 + MOTOR_ROTOR_OFFSET_ANGLE)
#define MOTOR_ROTOR_ANGLE_30					(20  + MOTOR_ROTOR_OFFSET_ANGLE)

// motor maximum rotation
// 700 is equal to 124 cadence, as TSDZ2 has a reduction ratio of 41.8
#define MAX_MOTOR_SPEED_ERPS					700 

// Set how often the motor speed limit controller runs in the isr
#define SPEED_CONTROLLER_CHECK_PERIODS			2000

// Set how oftern the current controller runs in the isr
#define CURRENT_CONTROLLER_CHECK_PERIODS		14

// adc measurements
// ------------------------------------------
// 10bit:	0.086V per step
//			0.156A per step
#define ADC_10BIT_VOLTAGE_PER_ADC_STEP_X512		44
#define ADC_10BIT_CURRENT_PER_ADC_STEP_X512		80

#define ADC_10BIT_STEPS_PER_VOLT_X512			5953


// filter coefficients
#define BATTERY_CURRENT_FILTER_COEFFICIENT		2
#define PHASE_CURRENT_FILTER_COEFFICIENT		2
#define BATTERY_VOLTAGE_FILTER_COEFFICIENT		2


#define SVM_TABLE_LEN							256
#define SVM_TABLE_MIDDLE						127
#define SIN_TABLE_LEN							60

 // motor states
#define BLOCK_COMMUTATION						1
#define SINEWAVE_INTERPOLATION_60_DEGREES		2


// index 0-256 to degrees 0-360
// table is -90 degree preadjusted
static const uint8_t svm_table[SVM_TABLE_LEN] = {
	  0,  11,  22,  32,  43,  54,  65,  75,  86,  96, 107, 117, 128, 138, 148, 158,
	168, 178, 188, 198, 207, 217, 222, 225, 228, 230, 233, 235, 238, 240, 242, 244,
	245, 247, 248, 250, 251, 252, 252, 253, 253, 254, 254, 254, 254, 254, 253, 253,
	252, 251, 250, 249, 247, 246, 244, 242, 241, 238, 236, 234, 231, 229, 226, 223,
	220, 223, 226, 229, 231, 234, 236, 238, 241, 242, 244, 246, 247, 249, 250, 251,
	252, 253, 253, 254, 254, 254, 254, 254, 253, 253, 252, 252, 251, 250, 248, 247,
	245, 244, 242, 240, 238, 235, 233, 230, 228, 225, 222, 217, 207, 198, 188, 178,
	168, 158, 148, 138, 128, 117, 107,  96,  86,  75,  65,  54,  43,  32,  22,  11,
	  0,  11,  22,  32,  43,  54,  65,  75,  86,  96, 107, 117, 128, 138, 148, 158,
	168, 178, 188, 198, 207, 217, 222, 225, 228, 230, 233, 235, 238, 240, 242, 244,
	245, 247, 248, 250, 251, 252, 252, 253, 253, 254, 254, 254, 254, 254, 253, 253,
	252, 251, 250, 249, 247, 246, 244, 242, 241, 238, 236, 234, 231, 229, 226, 223,
	220, 223, 226, 229, 231, 234, 236, 238, 241, 242, 244, 246, 247, 249, 250, 251,
	252, 253, 253, 254, 254, 254, 254, 254, 253, 253, 252, 252, 251, 250, 248, 247,
	245, 244, 242, 240, 238, 235, 233, 230, 228, 225, 222, 217, 207, 198, 188, 178,
	168, 158, 148, 138, 128, 117, 107, 96,   86,  75,  65,  54,  43,  32,  22,  11
};


static const uint8_t sin_table[SIN_TABLE_LEN] =
{
	  0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,
	 46,  49,  52,  54,  57,  60,  63,  66,  68,  71,  73,  76,  78,  81,  83,
	 86,  88,  90,  92,  95,  97,  99, 101, 102, 104, 106, 108, 109, 111, 113,
	114, 115, 117, 118, 119, 120, 121, 122, 123, 124, 125, 125, 126, 126, 127
};

// adc buffer access
// ------------------------------------------
#define ADC_10BIT_BATTERY_CURRENT			(ADC1->DB5RL)						// AIN5
#define ADC_10BIT_BATTERY_VOLTAGE			((ADC1->DB6RH << 8) | ADC1->DB6RL)	// AIN6

// adc current reading is truncated to 8bit since that allows a 
// range of up to 40A which it is not expected to be surpassed.
// this macro will check for 8bit overflow and is used to limit
// current in isr if overflow for some reason would occur.
#define ADC_10BIT_BATTERY_CURRENT_OVF		(ADC1->DB5RH != 0)


// motor control state (shared with isr)
// ------------------------------------------------------
#define CONTROL_STATE_DISABLE			0
#define CONTROL_STATE_PREPARE			1
#define CONTROL_STATE_START				2
#define CONTROL_STATE_RUNNING			3


static volatile uint8_t control_state = CONTROL_STATE_DISABLE;
static volatile bool is_lvc_triggered = false;
static volatile bool hall_sensor_error = false;

// not atomic, protected by disabling interrupt while read in compute_foc_angle
static volatile uint16_t speed_erps = 0; 

// current reading saved in 8 bits for atomic access, not expected to exceed 255 (40A)
static volatile uint8_t adc_battery_current = 0;	
static volatile uint8_t adc_phase_current = 0;
static volatile uint8_t adc_battery_target_current = 0;

static volatile uint8_t foc_angle = 0;

static volatile uint8_t pwm_duty_cycle = 0;
static volatile uint8_t pwm_duty_cycle_target = 0;

// calculated constant limits (from config)
static uint16_t adc_low_voltage_limit = 0;
static uint8_t adc_battery_max_current = 0;
static uint8_t adc_phase_max_current = 0;

// ------------------------------------------------------

// foc angle filter
static uint16_t foc_angle_accumulated = 0;

// battery voltage filter
static uint16_t adc_battery_voltage_accumulated = 0;
static uint16_t adc_battery_voltage_filtered = 0;

// battery current filter
static uint16_t adc_battery_current_accumulated = 0;
static uint16_t adc_battery_current_filtered = 0;

// motor phase current filter
static uint16_t adc_phase_current_accumulated = 0;
static uint16_t adc_phase_current_filtered = 0;

static uint16_t lvc_x10V = 0;
static uint8_t target_speed_percent = 0;
static uint8_t target_current_percent = 0;

static uint16_t adc_steps_per_volt_x512 = ADC_10BIT_STEPS_PER_VOLT_X512;


static void flash_opt2_afr5()
{
	// verify if PWM N channels are active on option bytes, if not, enable
	static const uint8_t Value = 0x20;

	if (OPT->OPT2 != Value)
	{
		// unlock data memory
		if (!(FLASH->IAPSR & FLASH_IAPSR_DUL))
		{
			FLASH->DUKR = FLASH_RASS_KEY2;
			FLASH->DUKR = FLASH_RASS_KEY1;

			while (!(FLASH->IAPSR & FLASH_IAPSR_DUL));
		}

		// Enable write access to option bytes 
		FLASH->CR2 |= FLASH_CR2_OPT;
		FLASH->NCR2 &= (uint8_t)(~FLASH_NCR2_NOPT);

		// program option byte and complement
		OPT->OPT2 = Value;
		OPT->NOPT2 = (uint8_t)(~Value);

		while (!(FLASH->IAPSR & FLASH_IAPSR_EOP));

		// Disable write access to option bytes
		FLASH->CR2 &= (uint8_t)(~FLASH_CR2_OPT);
		FLASH->NCR2 |= FLASH_NCR2_NOPT;

		// lock data memory
		FLASH->IAPSR &= ~FLASH_IAPSR_DUL;
	}
}

static void read_battery_voltage()
{
	// low pass filter the voltage readed value, to avoid possible fast spikes/noise
	adc_battery_voltage_accumulated -= adc_battery_voltage_accumulated >> BATTERY_VOLTAGE_FILTER_COEFFICIENT;
	adc_battery_voltage_accumulated += ADC_10BIT_BATTERY_VOLTAGE; // issue: no atomic access, adc reading is triggered from isr
	adc_battery_voltage_filtered = adc_battery_voltage_accumulated >> BATTERY_VOLTAGE_FILTER_COEFFICIENT;

	is_lvc_triggered = (adc_battery_voltage_filtered < adc_low_voltage_limit);
}

static void read_battery_current()
{
	// low pass filter the positive battery readed value (no regen current), to avoid possible fast spikes/noise
	adc_battery_current_accumulated -= adc_battery_current_accumulated >> BATTERY_CURRENT_FILTER_COEFFICIENT;
	adc_battery_current_accumulated += adc_battery_current;
	adc_battery_current_filtered = adc_battery_current_accumulated >> BATTERY_CURRENT_FILTER_COEFFICIENT;
}

static void read_phase_current()
{
	// low pass filter the positive motor pahse value (no regen current), to avoid possible fast spikes/noise
	adc_phase_current_accumulated -= adc_phase_current_accumulated >> PHASE_CURRENT_FILTER_COEFFICIENT;
	adc_phase_current_accumulated += adc_phase_current;
	adc_phase_current_filtered = adc_phase_current_accumulated >> PHASE_CURRENT_FILTER_COEFFICIENT;
}

static uint8_t asin_table(uint8_t inverted_angle_x128)
{
	// calc asin also converts the final result to degrees
	uint8_t index = 0;
	while (index < SIN_TABLE_LEN)
	{
		if (inverted_angle_x128 < sin_table[index])
		{
			break;
		}

		index++;
	}

	// first value of table is 0 so index will always increment to at least 1 and return 0
	return index--;
}

static void compute_foc_angle()
{
	uint16_t ui16_temp;
	uint32_t ui32_temp;
	uint16_t e_phase_voltage;
	uint32_t i_phase_current_x2;
	uint32_t l_x1048576;
	uint32_t w_angular_velocity_x16;
	uint16_t iwl_128;

	// FOC implementation by calculating the angle between phase current and rotor magnetic flux (BEMF)
	// 1. phase voltage is calculate
	// 2. I*w*L is calculated, where I is the phase current. L was a measured value for 48V motor.
	// 3. inverse sin is calculated of (I*w*L) / phase voltage, were we obtain the angle
	// 4. previous calculated angle is applied to phase voltage vector angle and so the
	// angle between phase current and rotor magnetic flux (BEMF) is kept at 0 (max torque per amp)

	// calc E phase voltage
	ui16_temp = adc_battery_voltage_filtered * ADC_10BIT_VOLTAGE_PER_ADC_STEP_X512;
	ui16_temp = (ui16_temp >> 8) * pwm_duty_cycle;
	e_phase_voltage = ui16_temp >> 9;

	// calc I phase current
	if (pwm_duty_cycle > 10)
	{
		ui16_temp = ((uint16_t)adc_battery_current_filtered) * ADC_10BIT_CURRENT_PER_ADC_STEP_X512;
		i_phase_current_x2 = ui16_temp / pwm_duty_cycle;
	}
	else
	{
		i_phase_current_x2 = 0;
	}

	// calc W angular velocity: erps * 6.3
	// 101 = 6.3 * 16
	TIM1->IER &= ~(uint8_t)TIM1_IT_CC4;
	ui16_temp = speed_erps;
	TIM1->IER |= TIM1_IT_CC4;
	w_angular_velocity_x16 = ui16_temp * 101;

	// ---------------------------------------------------------------------------------------------------------------------
	// 36 V motor: L = 76uH
	// 48 V motor: L = 135uH
	// ui32_l_x1048576 = 142; // 1048576 = 2^20 | 48V
	// ui32_l_x1048576 = 84; // 1048576 = 2^20 | 36V
	//
	// ui32_l_x1048576 = 142 <--- THIS VALUE WAS verified experimentaly on 2018.07 to be near the best value for a 48V motor
	// Test done with a fixed mechanical load, duty_cycle = 200 and 100 and measured battery current was 16 and 6 (10 and 4 amps)
	// ---------------------------------------------------------------------------------------------------------------------

#if 0
	l_x1048576 = 84; // 36 V motor
#else
	l_x1048576 = 142; // 48 V motor
#endif

	// calc IwL
	ui32_temp = i_phase_current_x2 * l_x1048576;
	ui32_temp *= w_angular_velocity_x16;
	iwl_128 = ui32_temp >> 18;

	// calc FOC angle
	uint8_t foc_angle_unfiltered = asin_table(iwl_128 / e_phase_voltage);

	// low pass filter FOC angle
	foc_angle_accumulated -= foc_angle_accumulated >> 4;
	foc_angle_accumulated += foc_angle_unfiltered;
	foc_angle = foc_angle_accumulated >> 4;
}


void motor_pre_init()
{
	SET_PIN_INPUT(PIN_HALL_SENSOR_A);
	SET_PIN_INPUT(PIN_HALL_SENSOR_B);
	SET_PIN_INPUT(PIN_HALL_SENSOR_C);

	SET_PIN_LOW(PIN_PWM_PHASE_A_LOW);
	SET_PIN_LOW(PIN_PWM_PHASE_A_HIGH);
	SET_PIN_LOW(PIN_PWM_PHASE_B_LOW);
	SET_PIN_LOW(PIN_PWM_PHASE_B_HIGH);
	SET_PIN_LOW(PIN_PWM_PHASE_C_LOW);
	SET_PIN_LOW(PIN_PWM_PHASE_C_HIGH);

	SET_PIN_OUTPUT(PIN_PWM_PHASE_A_LOW);
	SET_PIN_OUTPUT(PIN_PWM_PHASE_A_HIGH);
	SET_PIN_OUTPUT(PIN_PWM_PHASE_B_LOW);
	SET_PIN_OUTPUT(PIN_PWM_PHASE_B_HIGH);
	SET_PIN_OUTPUT(PIN_PWM_PHASE_C_LOW);	
	SET_PIN_OUTPUT(PIN_PWM_PHASE_C_HIGH);
}

void motor_init(uint16_t max_current_mA, uint8_t lvc_V, int16_t adc_calib_volt_step_offset)
{
	lvc_x10V = lvc_V * 10;

	uint32_t max_current_x10A = max_current_mA / 100;

	adc_steps_per_volt_x512 = ADC_10BIT_STEPS_PER_VOLT_X512 + adc_calib_volt_step_offset;

	// compute hard current limits (not changed after here)
	adc_battery_max_current = (uint8_t)(
		((((uint32_t)MIN(max_current_x10A, MAX_BATTERY_CURRENT_AMPS_X10)) * 512) / 10) / ADC_10BIT_CURRENT_PER_ADC_STEP_X512
	);

	adc_phase_max_current = (uint8_t)(
		((((uint32_t)MAX_MOTOR_PHASE_CURRENT_AMPS_X10) * 512) / 10) / ADC_10BIT_CURRENT_PER_ADC_STEP_X512
	);

	adc_low_voltage_limit = (uint16_t)((((uint32_t)lvc_V) * adc_steps_per_volt_x512) / 512);

	flash_opt2_afr5();
	timer1_init_motor_pwm();
	motor_disable();
}

void motor_process()
{
	read_battery_voltage();
	read_battery_current();
	read_phase_current();
	compute_foc_angle();
}


void motor_enable()
{
	if (control_state == CONTROL_STATE_DISABLE)
	{
		control_state = CONTROL_STATE_PREPARE;
	}
}

void motor_disable()
{
	control_state = CONTROL_STATE_DISABLE;
}

uint16_t motor_status()
{
	static uint16_t last_status = 0;

	uint16_t status = 0;
	if (hall_sensor_error)
		status |= MOTOR_ERROR_HALL_SENSOR;

	if (is_lvc_triggered)
		status |= MOTOR_ERROR_LVC;

	if (status != last_status)
	{
		last_status = status;
		eventlog_write_data(EVT_DATA_MOTOR_STATUS, status);
	}

	return status;
}

uint8_t motor_get_target_speed()
{
	return target_speed_percent;
}

uint8_t motor_get_target_current()
{
	return target_current_percent;
}


void motor_set_target_speed(uint8_t percent)
{
	if (percent > 100)
	{
		percent = 100;
	}

	if (percent != target_speed_percent)
	{
		target_speed_percent = percent;
		eventlog_write_data(EVT_DATA_TARGET_SPEED, percent);

		if (percent == 0)
		{
			pwm_duty_cycle_target = 0;
		}
		else
		{
			pwm_duty_cycle_target = (uint8_t)MAP16(percent, 1, 100, PWM_DUTY_CYCLE_MIN, PWM_DUTY_CYCLE_MAX);
		}
	}
}

void motor_set_target_current(uint8_t percent)
{
	if (percent > 100)
	{
		percent = 100;
	}

	if (percent != target_current_percent)
	{
		target_current_percent = percent;
		eventlog_write_data(EVT_DATA_TARGET_CURRENT, percent);

		adc_battery_target_current = ((uint16_t)percent * adc_battery_max_current) / 100;
	}
}

int16_t motor_calibrate_battery_voltage(uint16_t actual_voltage_x100)
{
	int16_t diff = 0;
	if (actual_voltage_x100 != 0)
	{
		uint16_t calibrated_adc_steps_volt_x512 = (uint16_t)(((uint32_t)adc_battery_voltage_filtered * 51200u) / actual_voltage_x100);

		diff = calibrated_adc_steps_volt_x512 - ADC_10BIT_STEPS_PER_VOLT_X512;
		adc_steps_per_volt_x512 = calibrated_adc_steps_volt_x512;
	}
	else
	{
		// reset calibration if 0 is received
		adc_steps_per_volt_x512 = ADC_10BIT_STEPS_PER_VOLT_X512;
		diff = 0;
	}

	eventlog_write_data(EVT_DATA_CALIBRATE_VOLTAGE, adc_steps_per_volt_x512);

	return diff;
}


uint16_t motor_get_battery_lvc_x10()
{
	return lvc_x10V;
}

uint16_t motor_get_battery_current_x10()
{
	return (uint16_t)((((uint32_t)adc_battery_current_filtered * 10) * ADC_10BIT_CURRENT_PER_ADC_STEP_X512) >> 9);
}

uint16_t motor_get_battery_voltage_x10()
{
	return (uint16_t)(((uint32_t)adc_battery_voltage_filtered * 5120) / adc_steps_per_volt_x512);
}


// state variables only used by isr
// ---------------------------------------------
static uint8_t hall_sensors_state_last = 0;
static uint8_t rotor_absolute_angle = 0;
static uint8_t half_erps_flag = 0;
static uint8_t commutation_type = BLOCK_COMMUTATION;

static uint16_t pwm_duty_cycle_ramp_up_counter = 0;
static uint16_t pwm_duty_cycle_ramp_down_counter = 0;

static uint16_t pwm_cycles_counter = 1;
static uint16_t pwm_cycles_counter_6 = 1;
static uint16_t pwm_cycles_counter_total = 0xffff;

static uint16_t adc_current_ramp_up_counter = 0;
static uint8_t current_controller_counter = 0;
static uint16_t speed_controller_counter = 0;

static uint8_t adc_battery_ramp_max_current = 0;

// Measures did with a 24V Q85 328 RPM motor, rotating motor backwards by hand:
// Hall sensor A positive to negative transition | BEMF phase B at max value / top of sinewave
// Hall sensor B positive to negative transition | BEMF phase A at max value / top of sinewave
// Hall sensor C positive to negative transition | BEMF phase C at max value / top of sinewave

// runs every 64us (PWM frequency)
// Measured on 2022-12-04, the interrupt code takes about 45% of the total 64us
void isr_timer1_cmp(void) __interrupt(ITC_IRQ_TIM1_CAPCOM)
{
	// read battery current adc value, should happen at middle
	// of the pwm duty cycle
	// disable scan mode
	ADC1->CR2 &= (uint8_t)(~ADC1_CR2_SCAN);

	// clear EOC flag and select channel 5 (current sense)
	ADC1->CSR = 0x05;

	// start ADC1 conversion
	ADC1->CR1 |= ADC1_CR1_ADON;
	while (!(ADC1->CSR & ADC1_FLAG_EOC));

	// atomic write (uint8), current is not expected to exceed adc 255 (40A)
	adc_battery_current = ADC_10BIT_BATTERY_CURRENT;

	switch (control_state)
	{
	case CONTROL_STATE_DISABLE:
		// disable outputs
		TIM1->CCER1 &= ~(uint8_t)(TIM1_CCER1_CC1E | TIM1_CCER1_CC1NE);	// OC1
		TIM1->CCER1 &= ~(uint8_t)(TIM1_CCER1_CC2E | TIM1_CCER1_CC2NE);	// OC2
		TIM1->CCER2 &= ~(uint8_t)(TIM1_CCER2_CC3E | TIM1_CCER2_CC3NE);	// OC3
		break;
	case CONTROL_STATE_PREPARE:
		if (speed_erps > 0)
		{
			// Restart from duty cycle mapped from erps.
			// This is probably not the correct way to do this, but
			// it seems to work reasonably well. VESC tracks back-emf
			// to calculate duty cyle to restart from...
			pwm_duty_cycle = (uint8_t)MAP32(speed_erps, 0, MAX_MOTOR_SPEED_ERPS, PWM_DUTY_CYCLE_MIN, PWM_DUTY_CYCLE_MAX);
		}	
		control_state = CONTROL_STATE_START;
		break;
	case CONTROL_STATE_START:
		// enable outputs
		TIM1->CCER1 |= (uint8_t)(TIM1_CCER1_CC1E | TIM1_CCER1_CC1NE); 	// OC1
		TIM1->CCER1 |= (uint8_t)(TIM1_CCER1_CC2E | TIM1_CCER1_CC2NE);	// OC2
		TIM1->CCER2 |= (uint8_t)(TIM1_CCER2_CC3E | TIM1_CCER2_CC3NE);	// OC3
		control_state = CONTROL_STATE_RUNNING;
		break;
	default:
		break;
	}

	// calculate motor current adc value
	if (pwm_duty_cycle > 0)
	{
		// atomic write (uint8), current is not expected to exceed adc 255 (40A)
		adc_phase_current = (uint8_t)((adc_battery_current * 256u) / pwm_duty_cycle);
	}
	else
	{
		adc_phase_current = 0;
	}

	// trigger adc conversion of all channels (scan conversion, buffered)
	ADC1->CR2 |= ADC1_CR2_SCAN; // enable scan mode
	ADC1->CSR = 0x07; // clear EOC flag first (selected also channel 7)
	ADC1->CR1 |= ADC1_CR1_ADON; // start ADC1 conversion


	// read hall sensor signals
	// find the motor rotor absolute angle
	// calc motor speed in erps (speed_erps)

	// read hall sensors signal pins and mask other pins
	// hall sensors sequence with motor forward rotation: 4, 6, 2, 3, 1, 5
	uint8_t hall_sensors_state = 
		((GET_PORT(PIN_HALL_SENSOR_A)->IDR & GET_PIN(PIN_HALL_SENSOR_A)) >> 5) |
		((GET_PORT(PIN_HALL_SENSOR_B)->IDR & GET_PIN(PIN_HALL_SENSOR_B)) >> 1) |
		((GET_PORT(PIN_HALL_SENSOR_C)->IDR & GET_PIN(PIN_HALL_SENSOR_C)) >> 3);

	// make sure we run next code only when there is a change on the hall sensors signal
	if (hall_sensors_state != hall_sensors_state_last)
	{
		hall_sensors_state_last = hall_sensors_state;

		switch (hall_sensors_state)
		{
		case 3:
			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_150;
			break;

		case 1:
			if (half_erps_flag == 1)
			{
				half_erps_flag = 0;
				pwm_cycles_counter_total = pwm_cycles_counter;
				pwm_cycles_counter = 1;

				if (pwm_cycles_counter_total > 0)
				{
					// This division takes 4.4us
					speed_erps = PWM_CYCLES_SECOND / pwm_cycles_counter_total;
				}
				else
				{
					speed_erps = PWM_CYCLES_SECOND;
				}

				// update motor commutation state based on motor speed
				if (speed_erps > MOTOR_ROTOR_ERPS_START_INTERPOLATION_60_DEGREES)
				{
					if (commutation_type == BLOCK_COMMUTATION)
					{
						commutation_type = SINEWAVE_INTERPOLATION_60_DEGREES;
					}
				}
				else
				{
					if (commutation_type == SINEWAVE_INTERPOLATION_60_DEGREES)
					{
						commutation_type = BLOCK_COMMUTATION;
						foc_angle = 0;
					}
				}
			}

			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_210;
			break;

		case 5:
			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_270;
			break;

		case 4:
			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_330;
			break;

		case 6:
			half_erps_flag = 1;

			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_30;
			break;

			// BEMF is always 90 degrees advanced over motor rotor position degree zero
			// and here (hall sensor C blue wire, signal transition from positive to negative),
			// phase B BEMF is at max value (measured on osciloscope by rotating the motor)
		case 2:
			rotor_absolute_angle = (uint8_t)MOTOR_ROTOR_ANGLE_90;
			break;

		default:
			// invalid hall sensor signal
			hall_sensor_error = true;
			return;
		}

		hall_sensor_error = false;
		pwm_cycles_counter_6 = 1;
	}

	// count number of fast loops / pwm cycles and reset some states when motor is near zero speed
	if (pwm_cycles_counter < PWM_CYCLES_COUNTER_MAX)
	{
		pwm_cycles_counter++;
		pwm_cycles_counter_6++;
	}
	else // happens when motor is stopped or near zero speed
	{
		pwm_cycles_counter = 1; // don't put to 0 to avoid 0 divisions
		pwm_cycles_counter_6 = 1;
		half_erps_flag = 0;
		speed_erps = 0;
		pwm_cycles_counter_total = 0xffff;
		foc_angle = 0;
		commutation_type = BLOCK_COMMUTATION;
		hall_sensors_state_last = 0; // this way we force execution of hall sensors code next time
	}

	// calc interpolation angle and sinewave table index
	uint8_t svm_table_index = rotor_absolute_angle + foc_angle;

#if 1 // may be useful to disable interpolation when debugging

	// calculate the interpolation angle (and it doesn't work when motor starts and at very low speeds)
	if (commutation_type == SINEWAVE_INTERPOLATION_60_DEGREES)
	{
		// division by 0: motor_pwm_cycles_counter_total should never be 0
		// TODO: verifiy if (motor_pwm_cycles_counter_6 << 8) do not overflow
		uint8_t interpolation_angle = (pwm_cycles_counter_6 << 8) / pwm_cycles_counter_total; // this operations take 4.4us
		svm_table_index += interpolation_angle;
	}
#endif


	// pwm duty cycle controller
	// ----------------------------------------------------------------------
	// brakes are active
	// limit battery undervoltage
	// limit battery max current
	// limit motor max erps
	// ramp up/down pwm duty cycle towards target

	++current_controller_counter;
	++speed_controller_counter;

	if	(
			control_state == CONTROL_STATE_DISABLE ||
			is_lvc_triggered ||
			(pwm_duty_cycle_target == 0) ||
			(GET_PIN_INPUT_STATE(PIN_BRAKE) == 0) //active low
		)
	{
		if (pwm_duty_cycle)
		{
			--pwm_duty_cycle;
		}
	}
	// do not control current at every PWM cycle, that will measure and control too fast. Use counter to limit
	else if
		(
			current_controller_counter > CURRENT_CONTROLLER_CHECK_PERIODS &&
			(
				// check if truncated 8bit current reading did overflow
				ADC_10BIT_BATTERY_CURRENT_OVF ||
				// compare against ramp controller current limit
				adc_battery_current > adc_battery_ramp_max_current ||
				// or hard motor phase current limit
				adc_phase_current > adc_phase_max_current
			)
		)
	{
		if (pwm_duty_cycle)
		{
			--pwm_duty_cycle;
		}
	}
	else if (
		speed_controller_counter > SPEED_CONTROLLER_CHECK_PERIODS && // test about every 100ms
		speed_erps > MAX_MOTOR_SPEED_ERPS
	)
	{
		if (pwm_duty_cycle)
		{
			--pwm_duty_cycle;
		}
	}
	else // nothing to limit, so adjust duty_cycle to duty_cycle_target
	{
		if (pwm_duty_cycle_target > pwm_duty_cycle)
		{
			if (pwm_duty_cycle_ramp_up_counter++ >= PWM_DUTY_CYCLE_RAMP_UP_INVERSE_STEP)
			{
				pwm_duty_cycle_ramp_up_counter = 0;
				++pwm_duty_cycle;
			}
		}
		else if (pwm_duty_cycle_target < pwm_duty_cycle)
		{
			if (pwm_duty_cycle_ramp_down_counter++ >= PWM_DUTY_CYCLE_RAMP_DOWN_INVERSE_STEP)
			{
				pwm_duty_cycle_ramp_down_counter = 0;
				--pwm_duty_cycle;
			}
		}
	}

	// reset periodic check counters
	if (speed_controller_counter > SPEED_CONTROLLER_CHECK_PERIODS)
	{
		speed_controller_counter = 0;
	}

	if (current_controller_counter > CURRENT_CONTROLLER_CHECK_PERIODS)
	{
		current_controller_counter = 0;
	}


	// calculate final pwm duty cycle values to be applied to TIMER1

	// The first half of the table is the positive offset from the middle (0x100),
	// in that case just set MSB to 0x1, and the value from the table*duty cycle to LSB.
	// The second half of the table is a negative offset from that same middle,
	// and should be substracted from 0x100.
	// To cheat, we leave it as 0x100 when this value * duty cycle is 0,
	// otherwise we assume MSB is 0, and just invert the value from the table from LSB.
	// Checking to see if svm_table_index >= 128 (180 degrees) by & 0x80,
	// as SDCC is not yet smart enough to do that automatically.
	#define CALC_PHASE(PHASE_OUTPUT) do {												\
		uint8_t tmp = ((uint16_t)(pwm_duty_cycle * svm_table[svm_table_index]) / 256);	\
		if (tmp > 0 && (svm_table_index & 0x80))										\
		{																				\
			PHASE_OUTPUT##_lsb = 0 - tmp;												\
			PHASE_OUTPUT##_msb = 0;														\
		}																				\
		else																			\
		{																				\
			PHASE_OUTPUT##_lsb = tmp;													\
			PHASE_OUTPUT##_msb = 1;														\
		}																				\
	} while (0)


	// phase B as reference phase
	uint8_t phase_b_voltage_msb;
	uint8_t phase_b_voltage_lsb;
	CALC_PHASE(phase_b_voltage);

	// phase C is advanced 120 degrees over phase B
	svm_table_index += 85; // 120º / 360 * 256 = 85
	uint8_t phase_c_voltage_msb;
	uint8_t phase_c_voltage_lsb;
	CALC_PHASE(phase_c_voltage);

	// phase A is advanced 240 degrees over phase B
	svm_table_index += 86; // 240º / 360 * 256 = 171 - 85 already added = 86
	uint8_t phase_a_voltage_msb;
	uint8_t phase_a_voltage_lsb;
	CALC_PHASE(phase_a_voltage);


	// set final duty cycle value to pwm timers
	// phase B
	TIM1->CCR3H = phase_b_voltage_msb;
	TIM1->CCR3L = phase_b_voltage_lsb;
	// phase C
	TIM1->CCR2H = phase_c_voltage_msb;
	TIM1->CCR2L = phase_c_voltage_lsb;
	// phase A
	TIM1->CCR1H = phase_a_voltage_msb;
	TIM1->CCR1L = phase_a_voltage_lsb;
	

	// ramp up motor current
	if (adc_battery_target_current > adc_battery_ramp_max_current)
	{
		if (adc_current_ramp_up_counter++ >= CURRENT_RAMP_UP_INVERSE_STEP)
		{
			adc_current_ramp_up_counter = 0;
			adc_battery_ramp_max_current++;
		}
	}
	else if (adc_battery_target_current < adc_battery_ramp_max_current)
	{
		// we are not doing a ramp down here, just directly setting to the target value
		adc_battery_ramp_max_current = adc_battery_target_current;
	}

	// clears the timer1 interrupt CC4 pending bit
	TIM1->SR1 = (uint8_t)(~(uint8_t)TIM1_IT_CC4);
}
