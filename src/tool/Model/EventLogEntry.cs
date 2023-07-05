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
		private const int EVT_MSG_CONFIG_READ_DONE =			2;
		private const int EVT_MSG_CONFIG_RESET =				3;
		private const int EVT_MSG_CONFIG_WRITE_DONE =			4;
		private const int EVT_MSG_CONFIG_READ_BEGIN =			5;
		private const int EVT_MSG_CONFIG_WRITE_BEGIN =			6;
		private const int EVT_MSG_PSTATE_READ_BEGIN =			7;
		private const int EVT_MSG_PSTATE_READ_DONE =			8;
		private const int EVT_MSG_PSTATE_WRITE_BEGIN =			9;
		private const int EVT_MSG_PSTATE_WRITE_DONE =			10;

		private const int EVT_ERROR_INIT_MOTOR =				64;
		private const int EVT_ERROR_CHANGE_TARGET_SPEED =		65;
		private const int EVT_ERROR_CHANGE_TARGET_CURRENT =		66;
		private const int EVT_ERROR_READ_MOTOR_STATUS =			67;
		private const int EVT_ERROR_READ_MOTOR_CURRENT =		68;
		private const int EVT_ERROR_READ_MOTOR_VOLTAGE =		69;

		private const int EVT_ERROR_EEPROM_READ =				70;
		private const int EVT_ERROR_EEPROM_WRITE =				71;
		private const int EVT_ERROR_EEPROM_ERASE =				72;
		private const int EVT_ERROR_EEPROM_VERIFY_VERSION =		73;
		private const int EVT_ERROR_EEPROM_VERIFY_CHECKSUM =	74;
		private const int EVT_ERROR_THROTTLE_LOW_LIMIT =		75;
		private const int EVT_ERROR_THROTTLE_HIGH_LIMIT =		76;
		private const int EVT_ERROR_WATCHDOG_TRIGGERED =		77;
		private const int EVT_ERROR_EXTCOM_CHECKSUM =			78;
		private const int EVT_ERROR_EXTCOM_DISCARD =			79;

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
		private const int EVT_DATA_MAIN_LOOP_TIME =				140;
		private const int EVT_DATA_THROTTLE_ADC =				141;
		private const int EVT_DATA_LVC_LIMITING =				142;
		private const int EVT_DATA_SHIFT_SENSOR =				143;
		private const int EVT_DATA_BBSHD_THERMISTOR =			144;
		private const int EVT_DATA_VOLTAGE =					145;
		private const int EVT_DATA_VOLTAGE_CALIBRATION =		146;
		private const int EVT_DATA_TORQUE_ADC =					147;
		private const int EVT_DATA_TORQUE_ADC_CALIBRATED =		148;


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
				case EVT_MSG_CONFIG_READ_DONE:
					return "Successfully read configuration from eeprom.";
				case EVT_MSG_CONFIG_RESET:
					Level = LogLevel.Warning;
					return "Configuration reset performed.";
				case EVT_MSG_CONFIG_WRITE_DONE:
					return "Configuration successfully written to eeprom.";
				case EVT_MSG_CONFIG_READ_BEGIN:
					return "Reading configuration from eeprom.";
				case EVT_MSG_CONFIG_WRITE_BEGIN:
					return "Writing configuration to eeprom.";
				case EVT_MSG_PSTATE_READ_BEGIN:
					return "Reading persisted state from eeprom.";
				case EVT_MSG_PSTATE_READ_DONE:
					return "Successfully read persisted state from eeprom.";
				case EVT_MSG_PSTATE_WRITE_BEGIN:
					return "Writing persisted stated to eeprom.";
				case EVT_MSG_PSTATE_WRITE_DONE:
					return "Persisted state successfully written to eeprom.";

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
				case EVT_ERROR_EEPROM_READ:
					return "Failed to read data from eeprom.";
				case EVT_ERROR_EEPROM_WRITE:
					return "Failed to write data to eeprom.";
				case EVT_ERROR_EEPROM_ERASE:
					return "Failed to erase eeprom before writing data.";
				case EVT_ERROR_EEPROM_VERIFY_VERSION:
					return "Data read from eeprom is of the wrong version.";
				case EVT_ERROR_EEPROM_VERIFY_CHECKSUM:
					return "Failed to verify checksum on data read from eeprom.";
				case EVT_ERROR_THROTTLE_LOW_LIMIT:
					return "Invalid throttle reading, below low limit, check throttle.";
				case EVT_ERROR_THROTTLE_HIGH_LIMIT:
					return "Invalid throttle reading, above high limit, check throttle.";
				case EVT_ERROR_WATCHDOG_TRIGGERED:
					return "Software reset by watchdog, software error.";
				case EVT_ERROR_EXTCOM_CHECKSUM:
					return "Message received with invalid checksum.";
				case EVT_ERROR_EXTCOM_DISCARD:
					return "Invalid message received on serial port, discarded.";

				case EVT_DATA_TARGET_CURRENT:
					return $"Motor target current changed to {_data}%.";
				case EVT_DATA_TARGET_SPEED:
					return $"Motor target speed changed to {_data}%.";
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
					{
						byte[] raw = BitConverter.GetBytes(_data.Value);
						return $"Temperature, motor={(sbyte)raw[1]}C, controller={(sbyte)raw[0]}C.";
					}
				case EVT_DATA_THERMAL_LIMITING:
					if (_data.Value != 0)
					{
						Level = LogLevel.Warning;
						return "Thermal limiting activated, reducing power.";
					}
					else
					{
						return "Thermal limiting deactivated.";
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
				case EVT_DATA_MAIN_LOOP_TIME:
					return $"Main loop, interval={_data}ms.";
				case EVT_DATA_THROTTLE_ADC:
					return $"Throttle adc, value={_data}.";
				case EVT_DATA_LVC_LIMITING:
					if (_data.Value != 0)
					{
						return $"Low voltage limiting activated, voltage={(_data / 100f):0.0}";
					}
					else
					{
						return "Low voltage limiting deactivated.";
					}
				case EVT_DATA_SHIFT_SENSOR:
					if (_data.Value != 0)
					{
						return $"Shift sensor power ramp started.";
					}
					else
					{
						return $"Shift sensor power ramp ended.";
					}
				case EVT_DATA_BBSHD_THERMISTOR:
					if (_data.Value != 0)
					{
						return "BBSHD motor with PTC thermistor detected.";
					}
					else
					{
						return "BBSHD motor with NTC thermistor detected.";
					}
				case EVT_DATA_VOLTAGE:
					return $"Battery voltage reading, value={_data / 100f}V.";
				case EVT_DATA_VOLTAGE_CALIBRATION:
					return $"Battery voltage calibration updated, adc_steps_per_volt={_data / 100f}.";
				case EVT_DATA_TORQUE_ADC:
					return $"Torque adc, value={_data}.";
				case EVT_DATA_TORQUE_ADC_CALIBRATED:
					return $"Torque sensor calibrated, adc_bias={_data}.";
			}

			if (_data.HasValue)
			{
				return $"Unknown ({_event}, value={_data.Value})";
			}

			return $"Unknown ({_event})";
		}


	}
}
