/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#include "app.h"
#include "fwconfig.h"
#include "cfgstore.h"
#include "motor.h"
#include "sensors.h"
#include "throttle.h"
#include "lights.h"
#include "uart.h"
#include "eventlog.h"
#include "util.h"
#include "system.h"


typedef struct
{
	assist_level_t level;

	// cached precomputed values
	// ---------------------------------

	// speed
	int32_t max_wheel_speed_rpm_x10;

	// pas
	uint8_t keep_current_target_percent;
	uint16_t keep_current_ramp_start_rpm_x10;
	uint16_t keep_current_ramp_end_rpm_x10;

} assist_level_data_t;

static uint8_t assist_level;
static uint8_t operation_mode;
static uint16_t global_speed_limit_rpm;
static int32_t global_throttle_speed_limit_rpm_x10;

static uint16_t lvc_voltage_x100;
static uint16_t lvc_ramp_down_start_voltage_x100;
static uint16_t lvc_ramp_down_end_voltage_x100;

static uint16_t full_voltage_range_x100;
static uint16_t padded_voltage_range_x100;
static uint16_t low_voltage_pad_x100;
static uint16_t high_voltage_pad_x100;

static assist_level_data_t assist_level_data;
static uint16_t speed_limit_ramp_interval_rpm_x10;

static bool cruise_paused;
static int8_t temperature_contr_c;
static int8_t temperature_motor_c;

static uint16_t ramp_up_current_interval_ms;
static uint32_t power_blocked_until_ms;

static uint16_t pretension_cutoff_speed_rpm_x10;

static bool lights_state = false;

void apply_pas_cadence(uint8_t* target_current, uint8_t throttle_percent);
#if HAS_TORQUE_SENSOR
void apply_pas_torque(uint8_t* target_current);
#endif

void apply_pretension(uint8_t* target_current);
void apply_cruise(uint8_t* target_current, uint8_t throttle_percent);
bool apply_throttle(uint8_t* target_current, uint8_t throttle_percent);
bool apply_speed_limit(uint8_t* target_current, uint8_t throttle_percent, bool pas_engaged, bool throttle_override);
bool apply_thermal_limit(uint8_t* target_current);
bool apply_low_voltage_limit(uint8_t* target_current);
bool apply_shift_sensor_interrupt(uint8_t* target_current);
bool apply_brake(uint8_t* target_current);
void apply_current_ramp_up(uint8_t* target_current, bool enable);
void apply_current_ramp_down(uint8_t* target_current, bool enable);

bool check_power_block();
void block_power_for(uint16_t ms);

void reload_assist_params();

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph);

void app_init()
{
	motor_disable();
	lights_disable();
	lights_set(g_config.lights_mode == LIGHTS_MODE_ALWAYS_ON);

	lvc_voltage_x100 = g_config.low_cut_off_v * 100u;

	full_voltage_range_x100 = EXPAND_U16(g_config.max_battery_x100v_u16h, g_config.max_battery_x100v_u16l) - lvc_voltage_x100;
	low_voltage_pad_x100 = full_voltage_range_x100 * BATTERY_EMPTY_OFFSET_PERCENT / 100;
	high_voltage_pad_x100 = full_voltage_range_x100 * BATTERY_FULL_OFFSET_PERCENT / 100;
	padded_voltage_range_x100 = full_voltage_range_x100 - low_voltage_pad_x100 - high_voltage_pad_x100;

	// The LVC ramp down end is at 0% battery, which is LVC + the low padding value.
	lvc_ramp_down_end_voltage_x100 = lvc_voltage_x100 + low_voltage_pad_x100;

	// The LVC ramp down starts at LVC_RAMP_DOWN_OFFSET_PERCENT battery, using the padded range.
	uint16_t lvc_ramp_down_offset_x100 = padded_voltage_range_x100 * LVC_RAMP_DOWN_OFFSET_PERCENT / 100;
	lvc_ramp_down_start_voltage_x100 = lvc_ramp_down_end_voltage_x100 + lvc_ramp_down_offset_x100;

	global_speed_limit_rpm = 0;
	global_throttle_speed_limit_rpm_x10 = 0;
	temperature_contr_c = 0;
	temperature_motor_c = 0;

	ramp_up_current_interval_ms = (g_config.max_current_amps * 10u) / g_config.current_ramp_amps_s;
	power_blocked_until_ms = 0;

	speed_limit_ramp_interval_rpm_x10 = convert_wheel_speed_kph_to_rpm(SPEED_LIMIT_RAMP_DOWN_INTERVAL_KPH) * 10;

	pretension_cutoff_speed_rpm_x10 = convert_wheel_speed_kph_to_rpm(g_config.pretension_speed_cutoff_kph) * 10;

	cruise_paused = true;
	operation_mode = OPERATION_MODE_DEFAULT;

	app_set_wheel_max_speed_rpm(convert_wheel_speed_kph_to_rpm(g_config.max_speed_kph));
	app_set_assist_level(g_config.assist_startup_level);
	reload_assist_params();

	if (g_config.assist_mode_select == ASSIST_MODE_SELECT_BRAKE_BOOT && brake_is_activated())
	{
		app_set_operation_mode(OPERATION_MODE_SPORT);
	}
}

void app_process()
{
	uint8_t target_current = 0;
	uint8_t target_cadence = assist_level_data.level.max_cadence_percent;
	uint8_t throttle_percent = throttle_map_response(throttle_read());

	bool pas_engaged = false;
	bool throttle_override = false;

	if (check_power_block())
	{
		target_current = 0;
	}
	else if (assist_level == ASSIST_PUSH && g_config.use_push_walk)
	{
		target_current = 10;
	}
	else
	{
		apply_pretension(&target_current);
		apply_pas_cadence(&target_current, throttle_percent);
#if HAS_TORQUE_SENSOR
		apply_pas_torque(&target_current);
#endif // HAS_TORQUE_SENSOR

		pas_engaged = target_current > 0;

		apply_cruise(&target_current, throttle_percent);

		throttle_override = apply_throttle(&target_current, throttle_percent);

		// override target cadence if configured in assist level
		if (throttle_override &&
			(assist_level_data.level.flags & ASSIST_FLAG_PAS) &&
			(assist_level_data.level.flags & ASSIST_FLAG_OVERRIDE_CADENCE))
		{
			target_cadence = THROTTLE_CADENCE_OVERRIDE_PERCENT;
		}
	}

	bool speed_limiting = apply_speed_limit(&target_current, throttle_percent, pas_engaged, throttle_override);
	bool thermal_limiting = apply_thermal_limit(&target_current);
	bool lvc_limiting = apply_low_voltage_limit(&target_current);
	bool shift_limiting =
#if HAS_SHIFT_SENSOR_SUPPORT
		apply_shift_sensor_interrupt(&target_current);
#else
		false;
#endif
	bool is_limiting = speed_limiting || thermal_limiting || lvc_limiting || shift_limiting;
	bool is_braking = apply_brake(&target_current);

	apply_current_ramp_up(&target_current, is_limiting || !throttle_override);
	apply_current_ramp_down(&target_current, !is_braking && !shift_limiting);

	motor_set_target_speed(target_cadence);
	motor_set_target_current(target_current);

	if (target_current > 0)
	{
		motor_enable();
	}
	else
	{
		motor_disable();
	}

	if (g_config.lights_mode == LIGHTS_MODE_DISABLED /*|| (motor_status() & MOTOR_ERROR_LVC) */)
	{
		lights_disable();
	}
	else
	{
		lights_enable();
	}
}


void app_set_assist_level(uint8_t level)
{
	if (assist_level != level)
	{
		if (assist_level == ASSIST_PUSH && g_config.use_push_walk)
		{
			// When releasig push walk mode pedals may have been rotating
			// with the motor, block motor power for 2 seconds to prevent PAS
			// sensor from incorrectly applying power if returning to a PAS level.
			block_power_for(1000);
		}

		assist_level = level;
		eventlog_write_data(EVT_DATA_ASSIST_LEVEL, assist_level);
		reload_assist_params();
	}
}

void app_set_lights(bool on)
{
	if ( // it's ok to write ugly code if you say it's ugly...
		(g_config.assist_mode_select == ASSIST_MODE_SELECT_LIGHTS) ||
		(assist_level == ASSIST_0 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS0_LIGHT) ||
		(assist_level == ASSIST_1 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS1_LIGHT) ||
		(assist_level == ASSIST_2 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS2_LIGHT) ||
		(assist_level == ASSIST_3 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS3_LIGHT) ||
		(assist_level == ASSIST_4 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS4_LIGHT) ||
		(assist_level == ASSIST_5 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS5_LIGHT) ||
		(assist_level == ASSIST_6 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS6_LIGHT) ||
		(assist_level == ASSIST_7 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS7_LIGHT) ||
		(assist_level == ASSIST_8 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS8_LIGHT) ||
		(assist_level == ASSIST_9 && g_config.assist_mode_select == ASSIST_MODE_SELECT_PAS9_LIGHT)
		)
	{
		if (on)
		{
			app_set_operation_mode(OPERATION_MODE_SPORT);
		}
		else
		{
			app_set_operation_mode(OPERATION_MODE_DEFAULT);
		}
	}
	else
	{
		if (g_config.lights_mode == LIGHTS_MODE_DEFAULT && lights_state != on)
		{
			lights_state = on;
			eventlog_write_data(EVT_DATA_LIGHTS, on);
			lights_set(on);
		}
	}
}

void app_set_operation_mode(uint8_t mode)
{
	if (operation_mode != mode)
	{
		operation_mode = mode;
		eventlog_write_data(EVT_DATA_OPERATION_MODE, operation_mode);
		reload_assist_params();
	}
}

void app_set_wheel_max_speed_rpm(uint16_t value)
{
	if (global_speed_limit_rpm != value)
	{
		global_speed_limit_rpm = value;
		global_throttle_speed_limit_rpm_x10 = ((int32_t)global_speed_limit_rpm *
			g_config.throttle_global_spd_lim_percent) / 10;

		eventlog_write_data(EVT_DATA_WHEEL_SPEED_PPM, value);
		reload_assist_params();
	}
}

uint8_t app_get_assist_level()
{
	return assist_level;
}

uint8_t app_get_lights()
{
	return lights_state;
}

uint8_t app_get_status_code()
{
	uint16_t motor = motor_status();

	if (motor & MOTOR_ERROR_HALL_SENSOR)
	{
		return STATUS_ERROR_HALL_SENSOR;
	}

	if (motor & MOTOR_ERROR_CURRENT_SENSE)
	{
		return STATUS_ERROR_CURRENT_SENSE;
	}

	if (motor & MOTOR_ERROR_POWER_RESET)
	{
		// Phase line error code reused, cause and meaning
		// of MOTOR_ERROR_POWER_RESET triggered on bbs02 is currently unknown
		return STATUS_ERROR_PHASE_LINE;
	}

	if (!throttle_ok())
	{
		return STATUS_ERROR_THROTTLE;
	}

	if (!torque_sensor_ok())
	{
		return STATUS_ERROR_TORQUE_SENSOR;
	}

	if (temperature_motor_c > MAX_TEMPERATURE)
	{
		return STATUS_ERROR_MOTOR_OVER_TEMP;
	}

	if (temperature_contr_c > MAX_TEMPERATURE)
	{
		return STATUS_ERROR_CONTROLLER_OVER_TEMP;
	}

	// Disable LVC error since it is not shown on display in original firmware
	// Uncomment if you want to enable
	// if (motor & MOTOR_ERROR_LVC)
	// {
	//     return STATUS_ERROR_LVC;
	// }

	if (brake_is_activated())
	{
		return STATUS_BRAKING;
	}

	return STATUS_NORMAL;
}

uint8_t app_get_temperature()
{
	int8_t temp_max = MAX(temperature_contr_c, temperature_motor_c);

	if (temp_max < 0)
	{
		return 0;
	}

	return (uint8_t)temp_max;
}

void apply_pretension(uint8_t* target_current)
{
	uint16_t current_speed_rpm_x10 = speed_sensor_get_rpm_x10();

	if (g_config.use_speed_sensor && g_config.use_pretension && current_speed_rpm_x10 > pretension_cutoff_speed_rpm_x10)
	{
		*target_current = 1;
	}
	return;
}

void apply_pas_cadence(uint8_t* target_current, uint8_t throttle_percent)
{
	if ((assist_level_data.level.flags & ASSIST_FLAG_PAS) && !(assist_level_data.level.flags & ASSIST_FLAG_PAS_TORQUE))
	{
		if (pas_is_pedaling_forwards() && pas_get_pulse_counter() > g_config.pas_start_delay_pulses)
		{
			if (assist_level_data.level.flags & ASSIST_FLAG_PAS_VARIABLE)
			{
				uint8_t current = (uint8_t)MAP16(throttle_percent, 0, 100, 0, assist_level_data.level.target_current_percent);
				if (current > *target_current)
				{
					*target_current = current;
				}
			}
			else
			{
				if (assist_level_data.level.target_current_percent > *target_current)
				{
					*target_current = assist_level_data.level.target_current_percent;
				}

				// apply "keep current" ramp
				if (g_config.pas_keep_current_percent < 100)
				{
					if (*target_current > assist_level_data.keep_current_target_percent &&
						pas_get_cadence_rpm_x10() > assist_level_data.keep_current_ramp_start_rpm_x10)
					{
						uint32_t cadence = MIN(pas_get_cadence_rpm_x10(), assist_level_data.keep_current_ramp_end_rpm_x10);

						// ramp down current towards keep_current_target_percent with rpm above keep_current_ramp_start_rpm_x10
						*target_current = MAP32(
							cadence,	// in
							assist_level_data.keep_current_ramp_start_rpm_x10,		// in_min
							assist_level_data.keep_current_ramp_end_rpm_x10,		// in_max
							*target_current,										// out_min
							assist_level_data.keep_current_target_percent);			// out_max
					}
				}
			}
		}
	}
}

#if HAS_TORQUE_SENSOR
void apply_pas_torque(uint8_t* target_current)
{
	if ((assist_level_data.level.flags & ASSIST_FLAG_PAS) && (assist_level_data.level.flags & ASSIST_FLAG_PAS_TORQUE))
	{
		if (pas_is_pedaling_forwards() && (pas_get_pulse_counter() > g_config.pas_start_delay_pulses || speed_sensor_is_moving()))
		{
			uint16_t torque_nm_x100 = torque_sensor_get_nm_x100();
			uint16_t cadence_rpm_x10 = pas_get_cadence_rpm_x10();
			if (cadence_rpm_x10 < TORQUE_POWER_LOWER_RPM_X10)
			{
				cadence_rpm_x10 = TORQUE_POWER_LOWER_RPM_X10;
			}

			uint16_t pedal_power_w_x10 = (uint16_t)(((uint32_t)torque_nm_x100 * cadence_rpm_x10) / 955);

			// used in division below to calculate target current,
			// clamp to 24V if no reading available (unexpected error).
			uint16_t battery_voltage_x10 = MAX(motor_get_battery_voltage_x10(), 240);

			uint16_t target_current_amp_x100 = (uint16_t)(((uint32_t)10 * pedal_power_w_x10 *
				assist_level_data.level.torque_amplification_factor_x10) / battery_voltage_x10);

			uint16_t max_current_amp_x100 = g_config.max_current_amps * 100;

			// limit target to ensure no overflow in map result
			if (target_current_amp_x100 > max_current_amp_x100)
			{
				target_current_amp_x100 = max_current_amp_x100;
			}
			uint8_t tmp_percent = (uint8_t)MAP32(target_current_amp_x100, 0, max_current_amp_x100, 0, 100);

			// minimum 1 percent current if pedaling
			if (tmp_percent < 1)
			{
				tmp_percent = 1;
			}
			// limit to maximum assist current for set level
			else if (tmp_percent > assist_level_data.level.target_current_percent)
			{
				tmp_percent = assist_level_data.level.target_current_percent;
			}

			if (tmp_percent > *target_current)
			{
				*target_current = tmp_percent;
			}
		}
	}
}
#endif

void apply_cruise(uint8_t* target_current, uint8_t throttle_percent)
{
	static bool cruise_block_throttle_return = false;

	if ((assist_level_data.level.flags & ASSIST_FLAG_CRUISE) && throttle_ok())
	{
		// pause cruise if brake activated
		if (brake_is_activated())
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// pause cruise if started pedaling backwards
		else if (pas_is_pedaling_backwards() && pas_get_pulse_counter() > CRUISE_DISENGAGE_PAS_PULSES)
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// pause cruise if throttle touched while cruise active
		else if (!cruise_paused && !cruise_block_throttle_return && throttle_percent > 0)
		{
			cruise_paused = true;
			cruise_block_throttle_return = true;
		}

		// unpause cruise if pedaling forward while engaging throttle > 50%
		else if (cruise_paused && !cruise_block_throttle_return && throttle_percent > 50 && pas_is_pedaling_forwards() && pas_get_pulse_counter() > CRUISE_ENGAGE_PAS_PULSES)
		{
			cruise_paused = false;
			cruise_block_throttle_return = true;
		}

		// reset flag tracking throttle to make sure throttle returns to idle position before engage/disenage cruise with throttle touch
		else if (cruise_block_throttle_return && throttle_percent == 0)
		{
			cruise_block_throttle_return = false;
		}

		if (cruise_paused)
		{
			*target_current = 0;
		}
		else
		{
			if (assist_level_data.level.target_current_percent > *target_current)
			{
				*target_current = assist_level_data.level.target_current_percent;
			}
		}
	}
}

bool apply_throttle(uint8_t* target_current, uint8_t throttle_percent)
{
	if ((assist_level_data.level.flags & ASSIST_FLAG_THROTTLE) && throttle_percent > 0 && throttle_ok())
	{
		uint8_t current = (uint8_t)MAP16(throttle_percent, 0, 100, g_config.throttle_start_percent, assist_level_data.level.max_throttle_current_percent);

		if (current >= *target_current)
		{
			*target_current = current;
			return true;
		}
	}

	return false;
}


bool apply_speed_limit(uint8_t* target_current, uint8_t throttle_percent, bool pas_engaged, bool throttle_override)
{
	static bool speed_limiting = false;

	if (!g_config.use_speed_sensor)
	{
		return false;
	}

	// global throttle speed limit applies if enabled in configuration, PAS is not engaged and throttle is used
	bool global_throttle_limit_active =
		!pas_engaged && 
		throttle_percent > 0 &&
		g_config.throttle_global_spd_lim_percent > 0 &&
		(
			g_config.throttle_global_spd_lim_opt == THROTTLE_GLOBAL_SPEED_LIMIT_ENABLED ||
			(g_config.throttle_global_spd_lim_opt == THROTTLE_GLOBAL_SPEED_LIMIT_STD_LVLS && operation_mode == OPERATION_MODE_DEFAULT)
		);

	bool throttle_speed_override_active = !global_throttle_limit_active && throttle_override &&
		(assist_level_data.level.flags & ASSIST_FLAG_PAS) &&
		(assist_level_data.level.flags & ASSIST_FLAG_OVERRIDE_SPEED);

	int32_t max_speed_rpm_x10;
	if (global_throttle_limit_active)
	{
		// use configured global throttle override speed limit
		max_speed_rpm_x10 = global_throttle_speed_limit_rpm_x10;
	}
	else if (throttle_speed_override_active)
	{
		// override assist level speed limit to global speed limit
		max_speed_rpm_x10 = global_speed_limit_rpm * 10;
	}
	else
	{
		// normal operation, use configured assist level speed limit
		max_speed_rpm_x10 = assist_level_data.max_wheel_speed_rpm_x10;
	}

	int32_t max_speed_ramp_low_rpm_x10 = max_speed_rpm_x10 - speed_limit_ramp_interval_rpm_x10;
	int32_t max_speed_ramp_high_rpm_x10 = max_speed_rpm_x10 + speed_limit_ramp_interval_rpm_x10;

	if (max_speed_rpm_x10 > 0)
	{
		int16_t current_speed_rpm_x10 = speed_sensor_get_rpm_x10();

		if (current_speed_rpm_x10 < max_speed_ramp_low_rpm_x10)
		{
			// no limiting
			if (speed_limiting)
			{
				speed_limiting = false;
				eventlog_write_data(EVT_DATA_SPEED_LIMITING, 0);
			}
		}
		else
		{
			if (!speed_limiting)
			{
				speed_limiting = true;
				eventlog_write_data(EVT_DATA_SPEED_LIMITING, 1);
			}

			if (current_speed_rpm_x10 > max_speed_ramp_high_rpm_x10)
			{
				if (*target_current > 1)
				{
					*target_current = 1;
					return true;
				}
			}
			else
			{
				// linear ramp down when approaching max speed.
				uint8_t tmp = (uint8_t)MAP32(current_speed_rpm_x10, max_speed_ramp_low_rpm_x10, max_speed_ramp_high_rpm_x10, *target_current, 1);
				if (*target_current > tmp)
				{
					*target_current = tmp;
					return true;
				}
			}
		}
	}

	return false;
}

bool apply_thermal_limit(uint8_t* target_current)
{
	static uint32_t next_log_temp_ms = 10000;

	static bool temperature_limiting = false;

	int16_t temp_contr_x100 = temperature_contr_x100();
	temperature_contr_c = temp_contr_x100 / 100;

	int16_t temp_motor_x100 = temperature_motor_x100();
	temperature_motor_c = temp_motor_x100 / 100;

	int16_t max_temp_x100 = MAX(temp_contr_x100, temp_motor_x100);
	int8_t max_temp = MAX(temperature_contr_c, temperature_motor_c);

	if (eventlog_is_enabled() && g_config.use_temperature_sensor && system_ms() > next_log_temp_ms)
	{
		next_log_temp_ms = system_ms() + 10000;
		eventlog_write_data(EVT_DATA_TEMPERATURE, (uint16_t)temperature_motor_c << 8 | temperature_contr_c);
	}

	if (max_temp >= (MAX_TEMPERATURE - MAX_TEMPERATURE_RAMP_DOWN_INTERVAL))
	{
		if (!temperature_limiting)
		{
			temperature_limiting = true;
			eventlog_write_data(EVT_DATA_THERMAL_LIMITING, 1);
		}

		if (max_temp_x100 > MAX_TEMPERATURE * 100)
		{
			max_temp_x100 = MAX_TEMPERATURE * 100;
		}

		uint8_t tmp = (uint8_t)MAP32(
			max_temp_x100,													// value
			(MAX_TEMPERATURE - MAX_TEMPERATURE_RAMP_DOWN_INTERVAL) * 100,	// in_min
			MAX_TEMPERATURE * 100,											// in_max
			100,															// out_min
			MAX_TEMPERATURE_LOW_CURRENT_PERCENT								// out_max
		);

		if (*target_current > tmp)
		{
			*target_current = tmp;
			return true;
		}
	}
	else
	{
		if (temperature_limiting)
		{
			temperature_limiting = false;
			eventlog_write_data(EVT_DATA_THERMAL_LIMITING, 0);
		}
	}

	return false;
}

bool apply_low_voltage_limit(uint8_t* target_current)
{
	static uint32_t next_log_volt_ms = 10000;
	static bool lvc_limiting = false;

	static uint32_t next_voltage_reading_ms = 125;
	static int32_t flt_min_bat_volt_x100 = 100 * 100;

	if (system_ms() > next_voltage_reading_ms)
	{
		next_voltage_reading_ms = system_ms() + 125;
		int32_t voltage_reading_x100 = motor_get_battery_voltage_x10() * 10ul;

		if (voltage_reading_x100 < flt_min_bat_volt_x100)
		{
			flt_min_bat_volt_x100 = EXPONENTIAL_FILTER(flt_min_bat_volt_x100, voltage_reading_x100, 8);
		}

		if (eventlog_is_enabled() && system_ms() > next_log_volt_ms)
		{
			next_log_volt_ms = system_ms() + 10000;
			eventlog_write_data(EVT_DATA_VOLTAGE, (uint16_t)voltage_reading_x100);
		}
	}

	uint16_t voltage_x100 = flt_min_bat_volt_x100;

	if (voltage_x100 <= lvc_ramp_down_start_voltage_x100)
	{
		if (!lvc_limiting)
		{
			eventlog_write_data(EVT_DATA_LVC_LIMITING, voltage_x100);
			lvc_limiting = true;
		}

		if (voltage_x100 < lvc_voltage_x100)
		{
			voltage_x100 = lvc_voltage_x100;
		}

		// Ramp down power until LVC_LOW_CURRENT_PERCENT when approaching LVC
		int8_t tmp = (int8_t)MAP32(
			voltage_x100,						// value
			lvc_ramp_down_end_voltage_x100,		// in_min
			lvc_ramp_down_start_voltage_x100,	// in_max
			LVC_LOW_CURRENT_PERCENT,			// out_min
			100									// out_max
		);
		uint8_t tmp2 = (uint8_t)CLAMP(tmp, 0, 100);

		if (*target_current > tmp2)
		{
			*target_current = tmp2;
			return true;
		}
	}

	return false;
}

#if HAS_SHIFT_SENSOR_SUPPORT
bool apply_shift_sensor_interrupt(uint8_t* target_current)
{
	static uint32_t shift_sensor_act_ms = 0;
	static bool shift_sensor_last = false;
	static bool shift_sensor_interrupting = false;
	static bool shift_sensor_logged = false;

	// Exit immediately if shift interrupts disabled.
	if (!g_config.use_shift_sensor)
	{
		return false;
	}

	bool active = shift_sensor_is_activated();
	if (active)
	{
		// Check for new pulse from the gear sensor during shift interrupt
		if (!shift_sensor_last && shift_sensor_interrupting)
		{
			// Consecutive gear change, do restart.
			shift_sensor_interrupting = false;
		}
		if (!shift_sensor_interrupting)
		{
			uint16_t duration_ms = EXPAND_U16(
				g_config.shift_interrupt_duration_ms_u16h,
				g_config.shift_interrupt_duration_ms_u16l
			);
			shift_sensor_act_ms = system_ms() + duration_ms;
			shift_sensor_interrupting = true;
		}
		shift_sensor_last = true;
	}
	else
	{
		shift_sensor_last = false;
	}

	if (!shift_sensor_interrupting)
	{
		return false;
	}

	if (system_ms() >= shift_sensor_act_ms)
	{
		// Shift is finished, reset function state.
		shift_sensor_interrupting = false;
		// Logging is skipped, unless current has been clamped during shift interrupt.
		if (shift_sensor_logged)
		{
			shift_sensor_logged = false;
			eventlog_write_data(EVT_DATA_SHIFT_SENSOR, 0);
		}
		return false;
	}

	if ((*target_current) > g_config.shift_interrupt_current_threshold_percent)
	{
		if (!shift_sensor_logged)
		{
			// Logging only once per shifting interrupt.
			shift_sensor_logged = true;
			eventlog_write_data(EVT_DATA_SHIFT_SENSOR, 1);
		}
		// Set target current based on desired current threshold during shift.
		*target_current = g_config.shift_interrupt_current_threshold_percent;

		return true;
	}

	return false;
}
#endif

bool apply_brake(uint8_t* target_current)
{
	bool is_braking = brake_is_activated();

	if (g_config.lights_mode == LIGHTS_MODE_BRAKE_LIGHT)
	{
		lights_set(is_braking);
	}

	if (is_braking)
	{
		*target_current = 0;
	}

	return is_braking;
}

void apply_current_ramp_up(uint8_t* target_current, bool enable)
{
	static uint8_t ramp_up_target_current = 0;
	static uint32_t last_ramp_up_increment_ms = 0;

	if (enable && *target_current > ramp_up_target_current)
	{
		uint32_t now = system_ms();
		uint16_t time_diff = now - last_ramp_up_increment_ms;

		if (time_diff >= ramp_up_current_interval_ms)
		{
			++ramp_up_target_current;

			if (last_ramp_up_increment_ms == 0)
			{
				last_ramp_up_increment_ms = now;
			}
			else
			{
				// offset for time overshoot to not accumulate large ramp error
				last_ramp_up_increment_ms = now - (uint8_t)(time_diff - ramp_up_current_interval_ms);
			}
		}

		*target_current = ramp_up_target_current;
	}
	else
	{
		ramp_up_target_current = *target_current;
		last_ramp_up_increment_ms = 0;
	}
}

void apply_current_ramp_down(uint8_t* target_current, bool enable)
{
	static uint8_t ramp_down_target_current = 0;
	static uint32_t last_ramp_down_decrement_ms = 0;

	// apply fast ramp down if coming from high target current (> 50%)
	if (enable && *target_current < ramp_down_target_current)
	{
		uint32_t now = system_ms();
		uint16_t time_diff = now - last_ramp_down_decrement_ms;

		if (time_diff >= 10)
		{
			uint8_t diff = ramp_down_target_current - *target_current;

			if (diff >= CURRENT_RAMP_DOWN_PERCENT_10MS)
			{
				ramp_down_target_current -= CURRENT_RAMP_DOWN_PERCENT_10MS;
			}
			else
			{
				ramp_down_target_current -= diff;
			}

			if (last_ramp_down_decrement_ms == 0)
			{
				last_ramp_down_decrement_ms = now;
			}
			else
			{
				// offset for time overshoot to not accumulate large ramp error
				last_ramp_down_decrement_ms = now - (uint8_t)(time_diff - 10);
			}
		}

		*target_current = ramp_down_target_current;
	}
	else
	{
		ramp_down_target_current = *target_current;
		last_ramp_down_decrement_ms = 0;
	}
}


bool check_power_block()
{
	if (power_blocked_until_ms != 0)
	{
		// power block is active, check if time to release
		if (system_ms() > power_blocked_until_ms)
		{
			power_blocked_until_ms = 0;
			return false;
		}

		return true;
	}

	return false;
}

void block_power_for(uint16_t ms)
{
	power_blocked_until_ms = system_ms() + ms;
}

void reload_assist_params()
{
	if (assist_level < ASSIST_PUSH)
	{
		assist_level_data.level = g_config.assist_levels[operation_mode][assist_level];

		assist_level_data.max_wheel_speed_rpm_x10 = ((int32_t)global_speed_limit_rpm * assist_level_data.level.max_speed_percent) / 10;

		if (assist_level_data.level.flags & ASSIST_FLAG_PAS)
		{
			assist_level_data.keep_current_target_percent = (uint8_t)((uint16_t)g_config.pas_keep_current_percent * assist_level_data.level.target_current_percent / 100);
			assist_level_data.keep_current_ramp_start_rpm_x10 = g_config.pas_keep_current_cadence_rpm * 10;
			assist_level_data.keep_current_ramp_end_rpm_x10 = (uint16_t)(((uint32_t)assist_level_data.level.max_cadence_percent * MAX_CADENCE_RPM_X10) / 100);
		}

		// pause cruise if swiching level
		cruise_paused = true;
	}
	// only apply push walk params if push walk is active in config,
	// otherwise data of previous assist level is kept.
	else if (assist_level == ASSIST_PUSH && g_config.use_push_walk)
	{
		assist_level_data.level.flags = 0;
		assist_level_data.level.target_current_percent = 0;
		assist_level_data.level.max_speed_percent = 0;
		assist_level_data.level.max_cadence_percent = 15;
		assist_level_data.level.max_throttle_current_percent = 0;

		assist_level_data.max_wheel_speed_rpm_x10 = convert_wheel_speed_kph_to_rpm(WALK_MODE_SPEED_KPH) * 10;
	}
}

uint16_t convert_wheel_speed_kph_to_rpm(uint8_t speed_kph)
{
	float radius_mm = EXPAND_U16(g_config.wheel_size_inch_x10_u16h, g_config.wheel_size_inch_x10_u16l) * 1.27f; // g_config.wheel_size_inch_x10 / 2.f * 2.54f;
	return (uint16_t)(25000.f / (3 * 3.14159f * radius_mm) * speed_kph);
}
