using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

namespace BBSFW.Model
{

	[XmlRoot("BBSFW", Namespace ="https://github.com/danielnilsson9/bbs-fw")]
	public class Configuration
	{
		public const int CurrentVersion = 4;
		public const int MinVersion = 1;
		public const int MaxVersion = CurrentVersion;

		public const int ByteSizeV1 = 120;
		public const int ByteSizeV2 = 124;
		public const int ByteSizeV3 = 149;
		public const int ByteSizeV4 = 154; // should be 2 more bytes compared to master

		public enum Feature
		{
			ShiftSensor,
			TorqueSensor,
			ControllerTemperatureSensor,
			MotorTemperatureSensor
		}

		public static int GetByteSize(int version)
		{
			switch (version)
			{
				case 1:
					return ByteSizeV1;
				case 2:
					return ByteSizeV2;
				case 3:
					return ByteSizeV3;
				case 4:
					return ByteSizeV4;
			}

			return 0;
		}

		public enum AssistModeSelect
		{
			Off = 0,
			Standard = 1,
			Lights = 2,
			Pas0AndLights = 3,
			Pas1AndLights = 4,
			Pas2AndLights = 5,
			Pas3AndLights = 6,
			Pas4AndLights = 7,
			Pas5AndLights = 8,
			Pas6AndLights = 9,
			Pas7AndLights = 10,
			Pas8AndLights = 11,
			Pas9AndLights = 12,
			BrakesOnBoot = 13
		}

		[Flags]
		public enum AssistFlagsType : byte
		{
			None = 0x00,
			Pas = 0x01,
			Throttle = 0x02,
			Cruise = 0x04,

			PasVariable = 0x08,
			PasTorque = 0x10,
			CadenceOverride = 0x20,
			SpeedOverride = 0x40
		};

		public enum ThrottleGlobalSpeedLimitOptions
		{
			Disabled = 0x00,
			Enabled = 0x01,
			StandardLevels = 0x02
		}

		public enum TemperatureSensor
		{
			Disabled = 0x00,
			Controller = 0x01,
			Motor = 0x02,
			All = 0x03
		}

		public enum WalkModeData
		{
			Speed = 0,
			Temperature = 1,
			RequestedPower = 2,
			BatteryPercent = 3
		}

		public enum LightsModeOptions
		{
			Default = 0,
			Disabled = 1,
			AlwaysOn = 2,
			BrakeLight = 3
		}

		public class AssistLevel
		{
			[XmlAttribute]
			public AssistFlagsType Type;

			[XmlAttribute]
			public uint MaxCurrentPercent;

			[XmlAttribute]
			public uint MaxThrottlePercent;

			[XmlAttribute]
			public uint MaxCadencePercent;

			[XmlAttribute]
			public uint MaxSpeedPercent;

			[XmlAttribute]
			public float TorqueAmplificationFactor;
		}

		[XmlIgnore]
		public BbsfwConnection.Controller Target { get; private set; }

		public uint MaxCurrentLimitAmps
		{
			get
			{
				switch (Target)
				{
					case BbsfwConnection.Controller.BBSHD:
						return 33;
					case BbsfwConnection.Controller.BBS02:
						return 30;
					case BbsfwConnection.Controller.TSDZ2:
						return 20;
				}

				return 50;
			}
		}

		// hmi
		[XmlIgnore]
		public bool UseFreedomUnits;

		// global
		public uint MaxCurrentAmps;
		public uint CurrentRampAmpsSecond;
		public float MaxBatteryVolts;
		public uint LowCutoffVolts;
		public uint MaxSpeedKph;

		// externals
		public bool UseSpeedSensor;
		public bool UseShiftSensor;
		public bool UsePushWalk;
		public TemperatureSensor UseTemperatureSensor;

		// lights
		public LightsModeOptions LightsMode;

		// pretension
		public bool UsePretension;
		public uint PretensionSpeedCutoffKph;

		// speed sensor
		public float WheelSizeInch;
		public uint NumWheelSensorSignals;
		
		// pas options
		public uint PasStartDelayPulses;
		public uint PasStopDelayMilliseconds;
		public uint PasKeepCurrentPercent;
		public uint PasKeepCurrentCadenceRpm;

		// throttle options
		public uint ThrottleStartMillivolts;
		public uint ThrottleEndMillivolts;
		public uint ThrottleStartPercent;
		public ThrottleGlobalSpeedLimitOptions ThrottleGlobalSpeedLimit;
		public uint ThrottleGlobalSpeedLimitPercent;

		// shift interrupt options
		public uint ShiftInterruptDuration;
		public uint ShiftInterruptCurrentThresholdPercent;

		// misc
		public WalkModeData WalkModeDataDisplay;

		// assists options
		public AssistModeSelect AssistModeSelection;
		public uint AssistStartupLevel;

		public AssistLevel[] StandardAssistLevels = new AssistLevel[10];
		public AssistLevel[] SportAssistLevels = new AssistLevel[10];

		public Configuration() : this(BbsfwConnection.Controller.Unknown)
		{ }

		public Configuration(BbsfwConnection.Controller target)
		{
			Target = target;

			UseFreedomUnits = Properties.Settings.Default.UseFreedomUnits;
			MaxCurrentAmps = 0;
			CurrentRampAmpsSecond = 0;
			MaxBatteryVolts = 0;
			LowCutoffVolts = 0;

			UseSpeedSensor = false;
			UseShiftSensor = false;
			UsePushWalk = false;
			UseTemperatureSensor = TemperatureSensor.All;

			LightsMode = LightsModeOptions.Default;

			UsePretension = false;
			PretensionSpeedCutoffKph = 0;

			WheelSizeInch = 0;
			NumWheelSensorSignals = 0;
			MaxSpeedKph = 0;

			PasStartDelayPulses = 0;
			PasStopDelayMilliseconds = 0;
			PasKeepCurrentPercent = 0;
			PasKeepCurrentCadenceRpm = 0;

			ThrottleStartMillivolts = 0;
			ThrottleEndMillivolts = 0;
			ThrottleStartPercent = 0;
			ThrottleGlobalSpeedLimit = ThrottleGlobalSpeedLimitOptions.Disabled;
			ThrottleGlobalSpeedLimitPercent = 0;

			ShiftInterruptDuration = 0;
			ShiftInterruptCurrentThresholdPercent = 0;

			WalkModeDataDisplay = WalkModeData.Speed;

			AssistModeSelection = AssistModeSelect.Off;
			AssistStartupLevel = 0;

			for (int i = 0; i < StandardAssistLevels.Length; ++i)
			{
				StandardAssistLevels[i] = new AssistLevel();
			}

			for (int i = 0; i < SportAssistLevels.Length; ++i)
			{
				SportAssistLevels[i] = new AssistLevel();
			}
		}

		public bool IsFeatureSupported(Feature feature)
		{
			if (Target == BbsfwConnection.Controller.Unknown)
			{
				return true;
			}

			switch (feature)
			{
				case Feature.ShiftSensor:
					return new[] { BbsfwConnection.Controller.BBSHD, BbsfwConnection.Controller.BBS02 }.Contains(Target);
				case Feature.TorqueSensor:
					return new[] { BbsfwConnection.Controller.TSDZ2 }.Contains(Target);
				case Feature.ControllerTemperatureSensor:
					return new[] { BbsfwConnection.Controller.BBSHD, BbsfwConnection.Controller.BBS02 }.Contains(Target);
				case Feature.MotorTemperatureSensor:
					return new[] { BbsfwConnection.Controller.BBSHD }.Contains(Target);
			}

			return false;
		}

		public bool ParseFromBufferV1(byte[] buffer)
		{
			if (buffer.Length != ByteSizeV1)
			{
				return false;
			}

			using (var s = new MemoryStream(buffer))
			{
				var br = new BinaryReader(s);

				UseFreedomUnits = br.ReadBoolean();

				MaxCurrentAmps = br.ReadByte();
				CurrentRampAmpsSecond = br.ReadByte();
				LowCutoffVolts = br.ReadByte();
				MaxSpeedKph = br.ReadByte();

				UseSpeedSensor = br.ReadBoolean();
				/* UseDisplay = */ br.ReadBoolean();
				UsePushWalk = br.ReadBoolean();

				WheelSizeInch = br.ReadUInt16() / 10f;
				NumWheelSensorSignals = br.ReadByte();

				PasStartDelayPulses = br.ReadByte();
				PasStopDelayMilliseconds = br.ReadByte() * 10u;

				ThrottleStartMillivolts = br.ReadUInt16();
				ThrottleEndMillivolts = br.ReadUInt16();
				ThrottleStartPercent = br.ReadByte();

				AssistModeSelection = (AssistModeSelect)br.ReadByte();
				AssistStartupLevel = br.ReadByte();

				for (int i = 0; i < StandardAssistLevels.Length; ++i)
				{
					StandardAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					StandardAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					StandardAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					StandardAssistLevels[i].MaxCadencePercent = br.ReadByte();
					StandardAssistLevels[i].MaxSpeedPercent = br.ReadByte();
				}

				for (int i = 0; i < SportAssistLevels.Length; ++i)
				{
					SportAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					SportAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					SportAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					SportAssistLevels[i].MaxCadencePercent = br.ReadByte();
					SportAssistLevels[i].MaxSpeedPercent = br.ReadByte();
				}
			}

			// apply default settings for non existing options in version
			MaxBatteryVolts = 0f;
			UseTemperatureSensor = TemperatureSensor.All;
			WalkModeDataDisplay = WalkModeData.Speed;
			PasKeepCurrentPercent = 100;
			PasKeepCurrentCadenceRpm = 255;
			UseShiftSensor = true;
			ShiftInterruptDuration = 600;
			ShiftInterruptCurrentThresholdPercent = 10;
			LightsMode = LightsModeOptions.Default;
			UsePretension = false;
			PretensionSpeedCutoffKph = 16;
			ThrottleGlobalSpeedLimit = ThrottleGlobalSpeedLimitOptions.Disabled;
			ThrottleGlobalSpeedLimitPercent = 100;

			return true;
		}

		public bool ParseFromBufferV2(byte[] buffer)
		{
			if (buffer.Length != ByteSizeV2)
			{
				return false;
			}

			using (var s = new MemoryStream(buffer))
			{
				var br = new BinaryReader(s);

				UseFreedomUnits = br.ReadBoolean();

				MaxCurrentAmps = br.ReadByte();
				CurrentRampAmpsSecond = br.ReadByte();
				MaxBatteryVolts = br.ReadUInt16() / 100f;
				LowCutoffVolts = br.ReadByte();
				MaxSpeedKph = br.ReadByte();

				UseSpeedSensor = br.ReadBoolean();
				/* UseDisplay = */ br.ReadBoolean();
				UsePushWalk = br.ReadBoolean();
				UseTemperatureSensor = (TemperatureSensor)br.ReadByte();

				WheelSizeInch = br.ReadUInt16() / 10f;
				NumWheelSensorSignals = br.ReadByte();

				PasStartDelayPulses = br.ReadByte();
				PasStopDelayMilliseconds = br.ReadByte() * 10u;
				PasKeepCurrentCadenceRpm = 255;
				PasKeepCurrentPercent = 100;

				ThrottleStartMillivolts = br.ReadUInt16();
				ThrottleEndMillivolts = br.ReadUInt16();
				ThrottleStartPercent = br.ReadByte();

				WalkModeDataDisplay = (WalkModeData)br.ReadByte();

				AssistModeSelection = (AssistModeSelect)br.ReadByte();
				AssistStartupLevel = br.ReadByte();

				for (int i = 0; i < StandardAssistLevels.Length; ++i)
				{
					StandardAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					StandardAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					StandardAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					StandardAssistLevels[i].MaxCadencePercent = br.ReadByte();
					StandardAssistLevels[i].MaxSpeedPercent = br.ReadByte();
				}

				for (int i = 0; i < SportAssistLevels.Length; ++i)
				{
					SportAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					SportAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					SportAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					SportAssistLevels[i].MaxCadencePercent = br.ReadByte();
					SportAssistLevels[i].MaxSpeedPercent = br.ReadByte();
				}
			}

			// apply default settings for non existing options in version
			PasKeepCurrentPercent = 100;
			PasKeepCurrentCadenceRpm = 255;
			UseShiftSensor = true;
			ShiftInterruptDuration = 600;
			ShiftInterruptCurrentThresholdPercent = 10;
			LightsMode = LightsModeOptions.Default;
			UsePretension = false;
			PretensionSpeedCutoffKph = 16;
			ThrottleGlobalSpeedLimit = ThrottleGlobalSpeedLimitOptions.Disabled;
			ThrottleGlobalSpeedLimitPercent = 100;

			return true;
		}

		public bool ParseFromBufferV3(byte[] buffer)
		{
			if (buffer.Length != ByteSizeV3)
			{
				return false;
			}

			using (var s = new MemoryStream(buffer))
			{
				var br = new BinaryReader(s);

				UseFreedomUnits = br.ReadBoolean();

				MaxCurrentAmps = br.ReadByte();
				CurrentRampAmpsSecond = br.ReadByte();
				MaxBatteryVolts = br.ReadUInt16() / 100f;
				LowCutoffVolts = br.ReadByte();
				MaxSpeedKph = br.ReadByte();

				UseSpeedSensor = br.ReadBoolean();
				UseShiftSensor = br.ReadBoolean();
				UsePushWalk = br.ReadBoolean();
				UsePretension = br.ReadBoolean();
				PretensionSpeedCutoffKph = br.ReadByte();
				UseTemperatureSensor = (TemperatureSensor)br.ReadByte();

				WheelSizeInch = br.ReadUInt16() / 10f;
				NumWheelSensorSignals = br.ReadByte();

				PasStartDelayPulses = br.ReadByte();
				PasStopDelayMilliseconds = br.ReadByte() * 10u;
				PasKeepCurrentPercent = br.ReadByte();
				PasKeepCurrentCadenceRpm = br.ReadByte();

				ThrottleStartMillivolts = br.ReadUInt16();
				ThrottleEndMillivolts = br.ReadUInt16();
				ThrottleStartPercent = br.ReadByte();

				ShiftInterruptDuration = br.ReadUInt16();
				ShiftInterruptCurrentThresholdPercent = br.ReadByte();

				WalkModeDataDisplay = (WalkModeData)br.ReadByte();

				AssistModeSelection = (AssistModeSelect)br.ReadByte();
				AssistStartupLevel = br.ReadByte();

				for (int i = 0; i < StandardAssistLevels.Length; ++i)
				{
					StandardAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					StandardAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					StandardAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					StandardAssistLevels[i].MaxCadencePercent = br.ReadByte();
					StandardAssistLevels[i].MaxSpeedPercent = br.ReadByte();
					StandardAssistLevels[i].TorqueAmplificationFactor = br.ReadByte() / 10f;
				}

				for (int i = 0; i < SportAssistLevels.Length; ++i)
				{
					SportAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					SportAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					SportAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					SportAssistLevels[i].MaxCadencePercent = br.ReadByte();
					SportAssistLevels[i].MaxSpeedPercent = br.ReadByte();
					SportAssistLevels[i].TorqueAmplificationFactor = br.ReadByte() / 10f;
				}
			}

			// apply default settings for non existing options in version
			LightsMode = LightsModeOptions.Default;
			ThrottleGlobalSpeedLimit = ThrottleGlobalSpeedLimitOptions.Disabled;
			ThrottleGlobalSpeedLimitPercent = 100;

			return true;
		}

		public bool ParseFromBufferV4(byte[] buffer)
		{
			if (buffer.Length != ByteSizeV4)
			{
				return false;
			}

			using (var s = new MemoryStream(buffer))
			{
				var br = new BinaryReader(s);

				UseFreedomUnits = br.ReadBoolean();

				MaxCurrentAmps = br.ReadByte();
				CurrentRampAmpsSecond = br.ReadByte();
				MaxBatteryVolts = br.ReadUInt16() / 100f;
				LowCutoffVolts = br.ReadByte();
				MaxSpeedKph = br.ReadByte();

				UseSpeedSensor = br.ReadBoolean();
				UseShiftSensor = br.ReadBoolean();
				UsePushWalk = br.ReadBoolean();
				UseTemperatureSensor = (TemperatureSensor)br.ReadByte();

				LightsMode = (LightsModeOptions)br.ReadByte();

				UsePretension = br.ReadBoolean();
				PretensionSpeedCutoffKph = br.ReadByte();

				WheelSizeInch = br.ReadUInt16() / 10f;
				NumWheelSensorSignals = br.ReadByte();

				PasStartDelayPulses = br.ReadByte();
				PasStopDelayMilliseconds = br.ReadByte() * 10u;
				PasKeepCurrentPercent = br.ReadByte();
				PasKeepCurrentCadenceRpm = br.ReadByte();

				ThrottleStartMillivolts = br.ReadUInt16();
				ThrottleEndMillivolts = br.ReadUInt16();
				ThrottleStartPercent = br.ReadByte();
				ThrottleGlobalSpeedLimit = (ThrottleGlobalSpeedLimitOptions)br.ReadByte();
				ThrottleGlobalSpeedLimitPercent = br.ReadByte();

				ShiftInterruptDuration = br.ReadUInt16();
				ShiftInterruptCurrentThresholdPercent = br.ReadByte();

				WalkModeDataDisplay = (WalkModeData)br.ReadByte();

				AssistModeSelection = (AssistModeSelect)br.ReadByte();
				AssistStartupLevel = br.ReadByte();

				for (int i = 0; i < StandardAssistLevels.Length; ++i)
				{
					StandardAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					StandardAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					StandardAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					StandardAssistLevels[i].MaxCadencePercent = br.ReadByte();
					StandardAssistLevels[i].MaxSpeedPercent = br.ReadByte();
					StandardAssistLevels[i].TorqueAmplificationFactor = br.ReadByte() / 10f;
				}

				for (int i = 0; i < SportAssistLevels.Length; ++i)
				{
					SportAssistLevels[i].Type = (AssistFlagsType)br.ReadByte();
					SportAssistLevels[i].MaxCurrentPercent = br.ReadByte();
					SportAssistLevels[i].MaxThrottlePercent = br.ReadByte();
					SportAssistLevels[i].MaxCadencePercent = br.ReadByte();
					SportAssistLevels[i].MaxSpeedPercent = br.ReadByte();
					SportAssistLevels[i].TorqueAmplificationFactor = br.ReadByte() / 10f;
				}
			}

			return true;
		}

		public byte[] WriteToBuffer()
		{
			using (var s = new MemoryStream())
			{
				var bw = new BinaryWriter(s);

				bw.Write(UseFreedomUnits);

				bw.Write((byte)MaxCurrentAmps);
				bw.Write((byte)CurrentRampAmpsSecond);
				bw.Write((UInt16)(MaxBatteryVolts * 100));
				bw.Write((byte)LowCutoffVolts);
				bw.Write((byte)MaxSpeedKph);

				bw.Write(UseSpeedSensor);
				bw.Write(UseShiftSensor);
				bw.Write(UsePushWalk);
				bw.Write((byte)UseTemperatureSensor);

				bw.Write((byte)LightsMode);

				bw.Write(UsePretension);
				bw.Write((byte)PretensionSpeedCutoffKph);

				bw.Write((UInt16)(WheelSizeInch * 10));
				bw.Write((byte)NumWheelSensorSignals);

				bw.Write((byte)PasStartDelayPulses);
				bw.Write((byte)(PasStopDelayMilliseconds / 10u));
				bw.Write((byte)PasKeepCurrentPercent);
				bw.Write((byte)PasKeepCurrentCadenceRpm);

				bw.Write((UInt16)ThrottleStartMillivolts);
				bw.Write((UInt16)ThrottleEndMillivolts);
				bw.Write((byte)ThrottleStartPercent);
				bw.Write((byte)ThrottleGlobalSpeedLimit);
				bw.Write((byte)ThrottleGlobalSpeedLimitPercent);

				bw.Write((UInt16)ShiftInterruptDuration);
				bw.Write((byte)ShiftInterruptCurrentThresholdPercent);

				bw.Write((byte)WalkModeDataDisplay);

				bw.Write((byte)AssistModeSelection);
				bw.Write((byte)AssistStartupLevel);

				for (int i = 0; i < StandardAssistLevels.Length; ++i)
				{
					bw.Write((byte)StandardAssistLevels[i].Type);
					bw.Write((byte)StandardAssistLevels[i].MaxCurrentPercent);
					bw.Write((byte)StandardAssistLevels[i].MaxThrottlePercent);
					bw.Write((byte)StandardAssistLevels[i].MaxCadencePercent);
					bw.Write((byte)StandardAssistLevels[i].MaxSpeedPercent);
					bw.Write((byte)Math.Round(StandardAssistLevels[i].TorqueAmplificationFactor * 10));
				}

				for (int i = 0; i < SportAssistLevels.Length; ++i)
				{
					bw.Write((byte)SportAssistLevels[i].Type);
					bw.Write((byte)SportAssistLevels[i].MaxCurrentPercent);
					bw.Write((byte)SportAssistLevels[i].MaxThrottlePercent);
					bw.Write((byte)SportAssistLevels[i].MaxCadencePercent);
					bw.Write((byte)SportAssistLevels[i].MaxSpeedPercent);
					bw.Write((byte)Math.Round(SportAssistLevels[i].TorqueAmplificationFactor * 10));
				}

				return s.ToArray();
			}
		}

		public void CopyFrom(Configuration cfg)
		{
			Target = cfg.Target;

			UseFreedomUnits = cfg.UseFreedomUnits;
			MaxCurrentAmps = cfg.MaxCurrentAmps;
			CurrentRampAmpsSecond = cfg.CurrentRampAmpsSecond;
			MaxBatteryVolts = cfg.MaxBatteryVolts;
			LowCutoffVolts = cfg.LowCutoffVolts;
			UseSpeedSensor = cfg.UseSpeedSensor;
			UseShiftSensor = cfg.UseShiftSensor;
			UsePushWalk = cfg.UsePushWalk;
			UseTemperatureSensor = cfg.UseTemperatureSensor;
			LightsMode = cfg.LightsMode;
			UsePretension = cfg.UsePretension;
			PretensionSpeedCutoffKph = cfg.PretensionSpeedCutoffKph;
			WheelSizeInch = cfg.WheelSizeInch;
			NumWheelSensorSignals = cfg.NumWheelSensorSignals;
			MaxSpeedKph = cfg.MaxSpeedKph;
			PasStartDelayPulses = cfg.PasStartDelayPulses;
			PasStopDelayMilliseconds = cfg.PasStopDelayMilliseconds;
			PasKeepCurrentPercent = cfg.PasKeepCurrentPercent;
			PasKeepCurrentCadenceRpm = cfg.PasKeepCurrentCadenceRpm;
			ThrottleStartMillivolts = cfg.ThrottleStartMillivolts;
			ThrottleEndMillivolts = cfg.ThrottleEndMillivolts;
			ThrottleStartPercent = cfg.ThrottleStartPercent;
			ThrottleGlobalSpeedLimit = cfg.ThrottleGlobalSpeedLimit;
			ThrottleGlobalSpeedLimitPercent = cfg.ThrottleGlobalSpeedLimitPercent;
			ShiftInterruptDuration = cfg.ShiftInterruptDuration;
			ShiftInterruptCurrentThresholdPercent = cfg.ShiftInterruptCurrentThresholdPercent;
			WalkModeDataDisplay = cfg.WalkModeDataDisplay;
			AssistModeSelection = cfg.AssistModeSelection;
			AssistStartupLevel = cfg.AssistStartupLevel;

			for (int i = 0; i < Math.Min(cfg.StandardAssistLevels.Length, StandardAssistLevels.Length); ++i)
			{
				StandardAssistLevels[i].Type = cfg.StandardAssistLevels[i].Type;
				StandardAssistLevels[i].MaxCurrentPercent = cfg.StandardAssistLevels[i].MaxCurrentPercent;
				StandardAssistLevels[i].MaxThrottlePercent = cfg.StandardAssistLevels[i].MaxThrottlePercent;
				StandardAssistLevels[i].MaxCadencePercent = cfg.StandardAssistLevels[i].MaxCadencePercent;
				StandardAssistLevels[i].MaxSpeedPercent = cfg.StandardAssistLevels[i].MaxSpeedPercent;
				StandardAssistLevels[i].TorqueAmplificationFactor = cfg.StandardAssistLevels[i].TorqueAmplificationFactor;
			}

			for (int i = 0; i < Math.Min(cfg.SportAssistLevels.Length, SportAssistLevels.Length); ++i)
			{
				SportAssistLevels[i].Type = cfg.SportAssistLevels[i].Type;
				SportAssistLevels[i].MaxCurrentPercent = cfg.SportAssistLevels[i].MaxCurrentPercent;
				SportAssistLevels[i].MaxThrottlePercent = cfg.SportAssistLevels[i].MaxThrottlePercent;
				SportAssistLevels[i].MaxCadencePercent = cfg.SportAssistLevels[i].MaxCadencePercent;
				SportAssistLevels[i].MaxSpeedPercent = cfg.SportAssistLevels[i].MaxSpeedPercent;
				SportAssistLevels[i].TorqueAmplificationFactor = cfg.SportAssistLevels[i].TorqueAmplificationFactor;
			}
		}

		public void ReadFromFile(string filepath)
		{
			var serializer = new XmlSerializer(typeof(Configuration));

			using (var reader = new FileStream(filepath, FileMode.Open))
			{
				var obj = serializer.Deserialize(reader) as Configuration;
				CopyFrom(obj);
			}
		}

		public void WriteToFile(string filepath)
		{
			var serializer = new XmlSerializer(typeof(Configuration));
			var settings = new XmlWriterSettings { Encoding = Encoding.UTF8, Indent = true };
			using (var xmlWriter = XmlWriter.Create(new StreamWriter(filepath), settings))
			{
				serializer.Serialize(xmlWriter, this);
			}
		}

		public void Validate()
		{
			ValidateLimits(MaxCurrentAmps, 5, MaxCurrentLimitAmps, "Max Current (A)");
			ValidateLimits(CurrentRampAmpsSecond, 1, 255, "Current Ramp (A/s)");
			ValidateLimits((uint)MaxBatteryVolts, 1, 100, "Max Battery Voltage (V)");
			ValidateLimits(LowCutoffVolts, 1, 100, "Low Voltage Cut Off (V)");

			ValidateLimits((uint)WheelSizeInch, 10, 40, "Wheel Size (inch)");
			ValidateLimits(NumWheelSensorSignals, 1, 10, "Wheel Sensor Signals");
			ValidateLimits(MaxSpeedKph, 0, 180, "Max Speed (km/h)");
			ValidateLimits(PretensionSpeedCutoffKph, 0, 100, "Pretension Speed Cutoff (km/h)");

			ValidateLimits(PasStartDelayPulses, 0, 24, "Pas Delay (pulses)");
			ValidateLimits(PasStopDelayMilliseconds, 50, 1000, "Pas Stop Delay (ms)");
			ValidateLimits(PasKeepCurrentPercent, 10, 100, "Pas Keep Current (%)");
			ValidateLimits(PasKeepCurrentCadenceRpm, 0, 255, "Pas Keep Current Cadence (rpm)");

			ValidateLimits(ThrottleStartMillivolts, 200, 2500, "Throttle Start (mV)");
			ValidateLimits(ThrottleEndMillivolts, 2500, 5000, "Throttle End (mV)");
			ValidateLimits(ThrottleStartPercent, 0, 100, "Throttle Start (%)");
			ValidateLimits(ThrottleGlobalSpeedLimitPercent, 0, 100, "Throttle Global Speed Limit (%)");

			ValidateLimits(ShiftInterruptDuration, 50, 2000, "Shift Interrupt Duration (ms)");
			ValidateLimits(ShiftInterruptCurrentThresholdPercent, 0, 100, "Shift Interrupt Current Threshold (%)");

			ValidateLimits(AssistStartupLevel, 0, 9, "Assist Startup Level");

			for (int i = 0; i < StandardAssistLevels.Length; ++i)
			{
				ValidateLimits(StandardAssistLevels[i].MaxCurrentPercent, 0, 100, $"Standard (Level {i}): Target Power (%)");
				ValidateLimits(StandardAssistLevels[i].MaxThrottlePercent, 0, 100, $"Standard (Level {i}): Max Throttle (%)");
				ValidateLimits(StandardAssistLevels[i].MaxCadencePercent, 0, 100, $"Standard (Level {i}): Max Cadence (%)");
				ValidateLimits(StandardAssistLevels[i].MaxSpeedPercent, 0, 100, $"Standard (Level {i}): Max Speed (%)");
				ValidateLimits((uint)StandardAssistLevels[i].TorqueAmplificationFactor, 0, 25, $"Standard (Level {i}): Torque Amplification");
			}

			for (int i = 0; i < SportAssistLevels.Length; ++i)
			{
				ValidateLimits(SportAssistLevels[i].MaxCurrentPercent, 0, 100, $"Sport (Level {i}): Target Power (%)");
				ValidateLimits(SportAssistLevels[i].MaxThrottlePercent, 0, 100, $"Sport (Level {i}): Max Throttle (%)");
				ValidateLimits(SportAssistLevels[i].MaxCadencePercent, 0, 100, $"Sport (Level {i}): Max Cadence (%)");
				ValidateLimits(SportAssistLevels[i].MaxSpeedPercent, 0, 100, $"Sport (Level {i}): Max Speed (%)");
				ValidateLimits((uint)SportAssistLevels[i].TorqueAmplificationFactor, 0, 25, $"Sport (Level {i}): Torque Amplification");
			}
		}

		private void ValidateLimits(uint value, uint min, uint max, string name)
		{
			if (value < min || value > max)
			{
				throw new Exception(name + " must be in interval " + min + "-" + max + ".");
			}
		}

	}
}
