using System;

namespace BBSFW.Model
{
	public class Configuration
	{
		public static int Version = 1;

		public enum AssistModeSelect
		{
			Off,
			Standard,
			Lights,
			Pas0AndLights
		}

		public enum AssistType
		{
			Disabled = 0x00,
			Pas = 0x01,
			Throttle = 0x02,
			PasAndThrottle = 0x03,
			Cruise = 0x04
		};


		public class AssistLevel
		{
			public AssistType flags;
			public uint MaxCurrentPercent;
			public uint MaxThrottlePercent;
			public uint MaxSpeedPercent;
		}

		// hmi
		public bool UseFreedomUnits;

		// power
		public uint MaxCurrentAmps;
		public uint LowCutoffVolts;

		// externals
		public bool UseSpeedSensor;
		public bool UseDisplay;
		public bool UsePushWalk;

		// speed sensor
		public float WheelSizeInch;
		public uint NumWheelSensorSignals;
		public uint MaxSpeedKph;

		// pas options
		public uint PasStartDelayPulses;
		public uint PasStopDelayMilliseconds;

		// throttle options
		public uint ThrottleStartMillivolts;
		public uint ThrottleEndMillivolts;
		public uint ThrottleStartPercent;

		// assists options
		public AssistModeSelect AssistModeSelection;
		public uint AssistStartupLevel;
		public AssistLevel[,] AssistLevels = new AssistLevel[2, 10];



		public Configuration()
		{
			LoadDefault();
		}

		public void LoadDefault()
		{
			UseFreedomUnits = false;
			MaxCurrentAmps = 30;
			LowCutoffVolts = 42;

			UseSpeedSensor = true;
			UseDisplay = true;
			UsePushWalk = true;

			WheelSizeInch = 28;
			NumWheelSensorSignals = 1;
			MaxSpeedKph = 60;

			PasStartDelayPulses = 3;
			PasStopDelayMilliseconds = 150;

			ThrottleStartMillivolts = 800;
			ThrottleEndMillivolts = 4200;
			ThrottleStartPercent = 10;

			AssistModeSelection = AssistModeSelect.Off;
			AssistStartupLevel = 3;

			for (int i = 0; i < AssistLevels.GetLength(0); ++i)
			{
				for (int j = 0; j < AssistLevels.GetLength(1); ++j)
				{
					AssistLevels[i, j] = new AssistLevel();
				}
			}

			// :TODO: assist levels
		}


		public void Validate()
		{
			ValidateLimits(MaxCurrentAmps, 5, 33, "Max Current (A)");
			ValidateLimits(LowCutoffVolts, 1, 100, "Low Volage Cut Off (V)");

			//ValidateLimits(WheelSizeInch, 1, 40, "Wheel Size (inch)");
			ValidateLimits(NumWheelSensorSignals, 1, 10, "Wheel Sensor Signals");
			ValidateLimits(MaxSpeedKph, 0, 100, "Max Speed (km/h)");

			ValidateLimits(PasStartDelayPulses, 0, 24, "Pas Delay (pulses)");
			ValidateLimits(PasStartDelayPulses, 50, 1000, "Pas Stop Delay (ms)");

			ValidateLimits(ThrottleStartMillivolts, 200, 2500, "Throttle Start (mV)");
			ValidateLimits(ThrottleEndMillivolts, 2500, 5000, "Throttle End (mV)");
			ValidateLimits(ThrottleStartPercent, 0, 100, "Throttle Start (%)");

			ValidateLimits(AssistStartupLevel, 0, 9, "Assist Startup Level");
		}


		private void ValidateLimits(uint value, uint min, uint max, string name)
		{
			if (value < min && value > max)
			{
				throw new Exception(name + " must be in interval " + min + "-" + max + ".");
			}
		}

	}
}
