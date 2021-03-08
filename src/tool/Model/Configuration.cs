using System;
using System.IO;
using System.Xml.Serialization;

namespace BBSFW.Model
{

	[XmlRoot("BBSFW", Namespace ="https://github.com/danielnilsson9/bbshd-fw")]
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
			[XmlAttribute]
			public AssistType Type;

			[XmlAttribute]
			public uint MaxCurrentPercent;

			[XmlAttribute]
			public uint MaxThrottlePercent;

			[XmlAttribute]
			public uint MaxSpeedPercent;
		}

		// hmi
		[XmlIgnore]
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


		public AssistLevel[] StandardAssistLevels = new AssistLevel[10];
		public AssistLevel[] SportAssistLevels = new AssistLevel[10];


		public Configuration()
		{
			LoadDefault();
		}

		public void LoadDefault()
		{
			UseFreedomUnits = Properties.Settings.Default.UseFreedomUnits;
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

			for(int i = 0; i < StandardAssistLevels.Length; ++i)
			{
				StandardAssistLevels[i] = new AssistLevel();
			}

			for (int i = 0; i < SportAssistLevels.Length; ++i)
			{
				SportAssistLevels[i] = new AssistLevel();
			}
			// :TODO: assist levels
		}

		public void ReadFromFile(string filepath)
		{
			var serializer = new XmlSerializer(typeof(Configuration));

			using (var reader = new FileStream(filepath, FileMode.Open))
			{
				var obj = serializer.Deserialize(reader) as Configuration;

				UseFreedomUnits = obj.UseFreedomUnits;
				MaxCurrentAmps = obj.MaxCurrentAmps;
				LowCutoffVolts = obj.LowCutoffVolts;
				UseSpeedSensor = obj.UseSpeedSensor;
				UseDisplay = obj.UseDisplay;
				UsePushWalk = obj.UsePushWalk;
				WheelSizeInch = obj.WheelSizeInch;
				NumWheelSensorSignals = obj.NumWheelSensorSignals;
				MaxSpeedKph = obj.MaxSpeedKph;
				PasStartDelayPulses = obj.PasStartDelayPulses;
				PasStopDelayMilliseconds = obj.PasStopDelayMilliseconds;
				ThrottleStartMillivolts = obj.ThrottleStartMillivolts;
				ThrottleEndMillivolts = obj.ThrottleEndMillivolts;
				ThrottleStartPercent = obj.ThrottleStartPercent;
				AssistModeSelection = obj.AssistModeSelection;
				AssistStartupLevel = obj.AssistStartupLevel;

				for (int i = 0; i < Math.Min(obj.StandardAssistLevels.Length, 10); ++i)
				{
					StandardAssistLevels[i].Type = obj.StandardAssistLevels[i].Type;
					StandardAssistLevels[i].MaxCurrentPercent = obj.StandardAssistLevels[i].MaxCurrentPercent;
					StandardAssistLevels[i].MaxSpeedPercent = obj.StandardAssistLevels[i].MaxSpeedPercent;
					StandardAssistLevels[i].MaxThrottlePercent = obj.StandardAssistLevels[i].MaxThrottlePercent;
				}

				for (int i = 0; i < Math.Min(obj.SportAssistLevels.Length, 10); ++i)
				{
					SportAssistLevels[i].Type = obj.SportAssistLevels[i].Type;
					SportAssistLevels[i].MaxCurrentPercent = obj.SportAssistLevels[i].MaxCurrentPercent;
					SportAssistLevels[i].MaxSpeedPercent = obj.SportAssistLevels[i].MaxSpeedPercent;
					SportAssistLevels[i].MaxThrottlePercent = obj.SportAssistLevels[i].MaxThrottlePercent;
				}
			}
		}

		public void WriteToFile(string filepath)
		{
			var serializer = new XmlSerializer(typeof(Configuration));
			using (var writer = new StreamWriter(filepath))
			{
				serializer.Serialize(writer, this);
			}
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
