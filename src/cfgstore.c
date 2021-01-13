#include "cfgstore.h"
#include "stc15.h"

#include <string.h>

static config_t __xdata config;


void load_default_config();

void cfgstore_init()
{
	load_default_config();

	// :TODO: restore from eeprom
	// verify version and checksum and reset to default on error
}



config_t* cfgstore_get()
{
	return &config;
}

bool cfgstore_save()
{
	// :TODO: save to eeprom with checksum
	return true;
}



void load_default_config()
{
	config.version = CONFIG_VERSION;

	config.max_current_amps = 30;
	config.low_voltage_cut_off = 24;		// :TODO: change!!!

	config.use_speed_sensor = 1;
	config.use_display = 1;
	config.use_push_walk = 1;

	config.wheel_size_inch_x10 = 280;
	config.speed_sensor_signals = 1;
	config.max_speed_kph = 40;

	config.pas_start_delay_pulses = 5;
	config.pas_stop_delay_ms = 200;

	config.throttle_start_voltage_mv = 1100;
	config.throttle_end_voltage_mv = 4000;
	config.throttle_start_percent = 10;

	config.assist_mode_select = ASSIST_MODE_SELECT_OFF;
	config.assist_startup_level = 3;

	memset(&config.assist_levels, 2 * 10, sizeof(assist_level_t));

	uint8_t current_limits[9] = { 25, 34, 43, 51, 60, 68, 74, 82, 90 };
	for (uint8_t i = 0; i < 9; ++i)
	{
		config.assist_levels[0][i+1].flags = ASSIST_FLAG_PAS | ASSIST_FLAG_THROTTLE;
		config.assist_levels[0][i+1].target_current_percent = current_limits[i];
		config.assist_levels[0][i+1].max_speed_percent = 100;
		config.assist_levels[0][i+1].max_throttle_current_percent = 100;
	}
}
