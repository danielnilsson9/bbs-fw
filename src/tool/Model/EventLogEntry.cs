using System;
using System.Collections.Generic;
using System.Text;

namespace BBSFW.Model
{
	public class EventLogEntry
	{
		private int _event;
		private int? _data;


		private const int EVT_MSG_MOTOR_INIT_OK =				1;
		private const int EVT_MSG_CONFIG_READ =					2;
		private const int EVT_MSG_CONFIG_RESET =				3;
		private const int EVT_MSG_CONFIG_WRITTEN=				4;

		private const int EVT_ERROR_INIT_MOTOR =				64;
		private const int EVT_ERROR_CHANGE_TARGET_SPEED =		65;
		private const int EVT_ERROR_CHANGE_TARGET_CURRENT =		66;
		private const int EVT_ERROR_READ_MOTOR_STATUS =			67;
		private const int EVT_ERROR_READ_MOTOR_CURRENT =		68;
		private const int EVT_ERROR_READ_MOTOR_VOLTAGE =		69;

		private const int EVT_ERROR_CONFIG_READ_EEPROM =		70;
		private const int EVT_ERROR_CONFIG_WRITE_EEPROM =		71;
		private const int EVT_ERROR_CONFIG_ERASE_EEPROM =		72;
		private const int EVT_ERROR_CONFIG_VERSION =			73;
		private const int EVT_ERROR_CONFIG_CHECKSUM =			74;

		private const int EVT_DATA_TARGET_CURRENT =				128;
		private const int EVT_DATA_TARGET_SPEED =				129;
		private const int EVT_DATA_MOTOR_STATUS =				130;
		private const int EVT_DATA_ASSIST_LEVEL =				131;
		private const int EVT_DATA_OPERATION_MODE =				132;
		private const int EVT_DATA_WHEEL_SPEED_PPM =			133;
		private const int EVT_DATA_LIGHTS =						134;
		private const int EVT_DATA_TEMPERATURE =				135;
		private const int EVT_DATA_THERMAL_LIMITING =			136;
		private const int EVT_DATA_SPEED_LIMITING =				137;
		private const int EVT_DATA_MAX_CURRENT_ADC_REQUEST =	138;
		private const int EVT_DATA_MAX_CURRENT_ADC_RESPONSE =	139;


		public enum LogLevel
		{
			Info,
			Warning,
			Error
		}


		public DateTime Timestamp { get; private set; }

		public LogLevel Level { get; private set; }

		public string Message { get; private set; }


		public EventLogEntry(int evt, int? data)
		{
			Timestamp = DateTime.Now;
			_event = evt;
			if (evt >= 64 & evt < 128)
			{
				Level = LogLevel.Error;
			}
			else
			{
				Level = LogLevel.Info;
			}

			_data = data;
			Message = Parse();
		}


		public string Parse()
		{
			switch (_event)
			{
				case EVT_MSG_MOTOR_INIT_OK:
					return "Motor initialization successful.";
				case EVT_MSG_CONFIG_READ:
					return "Successfully read configuration from eeprom.";
				case EVT_MSG_CONFIG_RESET:
					Level = LogLevel.Warning;
					return "Configuration reset performed.";
				case EVT_MSG_CONFIG_WRITTEN:
					return "Configuration written to eeprom.";

				case EVT_ERROR_INIT_MOTOR:
					return "Failed to perform motor controller initialization.";
				case EVT_ERROR_CHANGE_TARGET_CURRENT:
					return "Failed to set motor target current on motor controller.";
				case EVT_ERROR_CHANGE_TARGET_SPEED:
					return "Failed to set motor target speed on motor controller.";
				case EVT_ERROR_READ_MOTOR_STATUS:
					return "Failed to read status from motor controller.";
				case EVT_ERROR_READ_MOTOR_CURRENT:
					return "Failed to read current from motor controller.";
				case EVT_ERROR_READ_MOTOR_VOLTAGE:
					return "Failed to read voltage from motor controller.";
				case EVT_ERROR_CONFIG_READ_EEPROM:
					return "Failed to read config from eeprom.";
				case EVT_ERROR_CONFIG_WRITE_EEPROM:
					return "Failed to write config to eeprom.";
				case EVT_ERROR_CONFIG_ERASE_EEPROM:
					return "Failed to erase eeprom before writing config.";
				case EVT_ERROR_CONFIG_VERSION:
					return "Configuration read from eeprom is of the wrong version.";
				case EVT_ERROR_CONFIG_CHECKSUM:
					return "Failed to verify checksum on configuration read from eeprom.";

				case EVT_DATA_TARGET_CURRENT:
					return $"Motor target current changed to {_data}%.";
				case EVT_DATA_TARGET_SPEED:
					return $"Motor target speed changed to {_data * 100 / 256}%.";
				case EVT_DATA_MOTOR_STATUS:
					Level = _data != 0 ? LogLevel.Error : LogLevel.Info;
					return $"Motor controller status changed to 0x{_data:X}.";
				case EVT_DATA_ASSIST_LEVEL:
					return $"Assist level changed to {_data}.";
				case EVT_DATA_OPERATION_MODE:
					return $"Operation mode changed to {_data}.";
				case EVT_DATA_WHEEL_SPEED_PPM:
					return $"Max wheel speed changed to {_data} rpm.";
				case EVT_DATA_LIGHTS:
					return $"Lights status changed to {_data}.";
				case EVT_DATA_TEMPERATURE:
					return $"Motor controller temperature changed to {_data}C.";
				case EVT_DATA_THERMAL_LIMITING:
					if (_data.Value != 0)
					{
						Level = LogLevel.Warning;
						return "Thermal limit reached, power reduced to 50%.";
					}
					else
					{
						return "Thermal limiting removed.";
					}
				case EVT_DATA_SPEED_LIMITING:
					if (_data.Value != 0)
					{
						return "Speed limiting activated.";
					}
					else
					{
						return "Speed limiting deactivated.";
					}
				case EVT_DATA_MAX_CURRENT_ADC_REQUEST:
					return $"Requesting to configure max current on motor controller mcu, adc={_data}.";
				case EVT_DATA_MAX_CURRENT_ADC_RESPONSE:
					return $"Max current configured on motor controller mcu, response was adc={_data}.";
			}

			if (_data.HasValue)
			{
				return $"Unknown ({_data.Value})";
			}

			return "Unknown";
		}


	}
}
