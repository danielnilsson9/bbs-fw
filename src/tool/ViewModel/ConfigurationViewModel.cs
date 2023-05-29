using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Linq;

namespace BBSFW.ViewModel
{
	public class ConfigurationViewModel : ObservableObject
	{
		private Configuration _config;

		public static List<uint> PasStartDelayOptions { get; } =
			new List<uint>() {
				0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180,
				195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345, 360
			};

		public static List<Configuration.TemperatureSensor> TemperatureSensorOptions
		{
			get
			{
				return Enum.GetValues<Configuration.TemperatureSensor>().ToList();
			}
		}


		public static List<uint> StartupAssistLevelOptions { get; } =
			new List<uint>() { 0, 1, 2, 3, 4, 5, 6, 7 ,8, 9 };


		public static List<ValueItemViewModel<Configuration.AssistModeSelect>> AssistModeSelectOptions { get; } =
			new List<ValueItemViewModel<Configuration.AssistModeSelect>>
			{
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Off, "Off"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Standard, "Sport Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Lights, "Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.BrakesOnBoot, "Brakes @ Power On"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas0AndLights, "PAS 0 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas1AndLights, "PAS 1 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas2AndLights, "PAS 2 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas3AndLights, "PAS 3 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas4AndLights, "PAS 4 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas5AndLights, "PAS 5 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas6AndLights, "PAS 6 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas7AndLights, "PAS 7 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas8AndLights, "PAS 8 + Lights Button"),
				new ValueItemViewModel<Configuration.AssistModeSelect>(Configuration.AssistModeSelect.Pas9AndLights, "PAS 9 + Lights Button"),
			};

		public static List<ValueItemViewModel<Configuration.WalkModeData>> WalkModeDataDisplayOptions { get; } =
			new List<ValueItemViewModel<Configuration.WalkModeData>>
			{
				new ValueItemViewModel<Configuration.WalkModeData>(Configuration.WalkModeData.Speed, "Speed"),
				new ValueItemViewModel<Configuration.WalkModeData>(Configuration.WalkModeData.Temperature, "Temperature (C)"),
				new ValueItemViewModel<Configuration.WalkModeData>(Configuration.WalkModeData.RequestedPower, "Requested Power (%)"),
				new ValueItemViewModel<Configuration.WalkModeData>(Configuration.WalkModeData.BatteryPercent, "Battery Level (%)")
			};

		public static List<ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit>> ThrottleGlobalSpeedLimitOptions { get; } =
			new List<ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit>>
			{
				new ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit>(Configuration.ThrottleGlobalSpeedLimit.Disabled, "Disabled"),
				new ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit>(Configuration.ThrottleGlobalSpeedLimit.Enabled, "Enabled"),
				new ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit>(Configuration.ThrottleGlobalSpeedLimit.StandardLevels, "Standard Levels"),
			};


		// support 

		public bool IsTorqueSensorSupported
		{
			get { return _config.IsFeatureSupported(Configuration.Feature.TorqueSensor); }
		}

		public bool IsShiftSensorSupported
		{
			get { return _config.IsFeatureSupported(Configuration.Feature.ShiftSensor); }
		}


		// configuration

		public bool UseMetricUnits
		{
			get
			{
				return !_config.UseFreedomUnits;
			}
			set
			{
				if (_config.UseFreedomUnits == value)
				{
					_config.UseFreedomUnits = !value;

					Properties.Settings.Default.UseFreedomUnits = _config.UseFreedomUnits;
					Properties.Settings.Default.Save();

					OnPropertyChanged(nameof(UseImperialUnits));
					OnPropertyChanged(nameof(UseMetricUnits));
				}
			}
		}

		public bool UseImperialUnits
		{
			get
			{
				return _config.UseFreedomUnits;
			}
			set
			{
				if (_config.UseFreedomUnits != value)
				{
					_config.UseFreedomUnits = value;

					Properties.Settings.Default.UseFreedomUnits = _config.UseFreedomUnits;
					Properties.Settings.Default.Save();

					OnPropertyChanged(nameof(UseImperialUnits));
					OnPropertyChanged(nameof(UseMetricUnits));
				}
			}
		}



		public uint MaxCurrentAmps
		{
			get { return _config.MaxCurrentAmps; }
			set
			{
				if (_config.MaxCurrentAmps != value)
				{
					_config.MaxCurrentAmps = value;
					OnPropertyChanged(nameof(MaxCurrentAmps));
				}
			}
		}

		public uint CurrentRampAmpsSecond
		{
			get { return _config.CurrentRampAmpsSecond; }
			set
			{
				if (_config.CurrentRampAmpsSecond != value)
				{
					_config.CurrentRampAmpsSecond = value;
					OnPropertyChanged(nameof(CurrentRampAmpsSecond));
				}
			}
		}

		public float MaxBatteryVolts
		{
			get { return _config.MaxBatteryVolts; }
			set
			{
				if (_config.MaxBatteryVolts != value)
				{
					_config.MaxBatteryVolts = value;
					OnPropertyChanged(nameof(MaxBatteryVolts));
				}
			}
		}

		public uint LowCutoffVolts
		{
			get { return _config.LowCutoffVolts; }
			set
			{
				if (_config.LowCutoffVolts != value)
				{
					_config.LowCutoffVolts = value;
					OnPropertyChanged(nameof(LowCutoffVolts));
				}
			}
		}

		public uint MaxSpeedKph
		{
			get { return _config.MaxSpeedKph; }
			set
			{
				if (_config.MaxSpeedKph != value)
				{
					_config.MaxSpeedKph = value;
					OnPropertyChanged(nameof(MaxSpeedKph));
					OnPropertyChanged(nameof(MaxSpeedMph));
				}
			}
		}

		public uint MaxSpeedMph
		{
			get { return KphToMph(_config.MaxSpeedKph); }
			set
			{
				if (_config.MaxSpeedKph != MphToKph(value))
				{
					_config.MaxSpeedKph = MphToKph(value);
					OnPropertyChanged(nameof(MaxSpeedKph));
					OnPropertyChanged(nameof(MaxSpeedMph));
				}
			}
		}

		public bool UseSpeedSensor
		{
			get { return _config.UseSpeedSensor; }
			set
			{
				if (_config.UseSpeedSensor != value)
				{
					_config.UseSpeedSensor = value;
					OnPropertyChanged(nameof(UseSpeedSensor));
				}
			}
		}

		public bool UseShiftSensor
		{
			get { return _config.UseShiftSensor; }
			set
			{
				if (_config.UseShiftSensor != value)
				{
					_config.UseShiftSensor = value;
					OnPropertyChanged(nameof(UseShiftSensor));
				}
			}
		}

		public bool UsePushWalk
		{
			get { return _config.UsePushWalk; }
			set
			{
				if (_config.UsePushWalk != value)
				{
					_config.UsePushWalk = value;
					OnPropertyChanged(nameof(UsePushWalk));
				}
			}
		}

		public Configuration.TemperatureSensor UseTemperatureSensor
		{
			get { return _config.UseTemperatureSensor; }
			set
			{
				if (_config.UseTemperatureSensor != value)
				{
					_config.UseTemperatureSensor = value;
					OnPropertyChanged(nameof(UseTemperatureSensor));
				}
			}
		}

		public bool LightsAlwaysOn
		{
			get { return _config.LightsAlwaysOn; }
			set
			{
				if (_config.LightsAlwaysOn != value)
				{
					_config.LightsAlwaysOn = value;
					OnPropertyChanged(nameof(LightsAlwaysOn));
				}
			}
		}

		public uint ThrottleStartVoltageMillivolts
		{
			get { return _config.ThrottleStartMillivolts; }
			set
			{
				if (_config.ThrottleStartMillivolts != value)
				{
					_config.ThrottleStartMillivolts = value;
					OnPropertyChanged(nameof(ThrottleStartVoltageMillivolts));
				}
			}
		}

		public uint ThrottleEndVoltageMillivolts
		{
			get { return _config.ThrottleEndMillivolts; }
			set
			{
				if (_config.ThrottleEndMillivolts != value)
				{
					_config.ThrottleEndMillivolts = value;
					OnPropertyChanged(nameof(ThrottleEndVoltageMillivolts));
				}
			}
		}

		public uint ThrottleStartCurrentPercent
		{
			get { return _config.ThrottleStartPercent; }
			set
			{
				if (_config.ThrottleStartPercent != value)
				{
					_config.ThrottleStartPercent = value;
					OnPropertyChanged(nameof(ThrottleStartCurrentPercent));
				}
			}
		}

		public ValueItemViewModel<Configuration.ThrottleGlobalSpeedLimit> ThrottleGlobalSpeedLimitOpt
		{
			get
			{
				return ThrottleGlobalSpeedLimitOptions.FirstOrDefault((e) => e.Value == _config.ThrottleGlobalSpeedLimitOpt);
			}
			set
			{
				if (_config.ThrottleGlobalSpeedLimitOpt != value.Value)
				{
					_config.ThrottleGlobalSpeedLimitOpt = value.Value;
					OnPropertyChanged(nameof(ThrottleGlobalSpeedLimitOpt));
				}
			}
		}

		public uint ThrottleGlobalSpeedLimitPercent
		{
			get { return _config.ThrottleGlobalSpeedLimitPercent; }
			set
			{
				if (_config.ThrottleGlobalSpeedLimitPercent != value)
				{
					_config.ThrottleGlobalSpeedLimitPercent = value;
					OnPropertyChanged(nameof(ThrottleGlobalSpeedLimitPercent));
				}
			}
		}



		public uint PasStartDelayDegrees
		{
			get { return _config.PasStartDelayPulses * 15; }
			set
			{
				if (_config.PasStartDelayPulses * 15 != value)
				{
					_config.PasStartDelayPulses = value / 15;
					OnPropertyChanged(nameof(PasStartDelayDegrees));
				}
			}
		}

		public uint PasStopDelayMilliseconds
		{
			get { return _config.PasStopDelayMilliseconds; }
			set
			{
				if (_config.PasStopDelayMilliseconds != value)
				{
					_config.PasStopDelayMilliseconds = value;
					OnPropertyChanged(nameof(PasStopDelayMilliseconds));
				}
			}
		}

		public uint PasKeepCurrentPercent
		{
			get { return _config.PasKeepCurrentPercent; }
			set
			{
				if (_config.PasKeepCurrentPercent != value)
				{
					_config.PasKeepCurrentPercent = value;
					OnPropertyChanged(nameof(PasKeepCurrentPercent));
				}
			}
		}

		public uint PasKeepCurrentCadenceRpm
		{
			get { return _config.PasKeepCurrentCadenceRpm; }
			set
			{
				if (_config.PasKeepCurrentCadenceRpm != value)
				{
					_config.PasKeepCurrentCadenceRpm = value;
					OnPropertyChanged(nameof(PasKeepCurrentCadenceRpm));
				}
			}
		}

		public float WheelSizeInch
		{
			get { return _config.WheelSizeInch; }
			set
			{
				if (_config.WheelSizeInch != value)
				{
					_config.WheelSizeInch = value;
					OnPropertyChanged(nameof(WheelSizeInch));
				}
			}
		}

		public uint SpeedSensorSignals
		{
			get { return _config.NumWheelSensorSignals; }
			set
			{
				if (_config.NumWheelSensorSignals != value)
				{
					_config.NumWheelSensorSignals = value;
					OnPropertyChanged(nameof(SpeedSensorSignals));
				}
			}
		}

		public uint ShiftInterruptDuration
		{
			get { return _config.ShiftInterruptDuration; }
			set
			{
				if (_config.ShiftInterruptDuration != value)
				{
					_config.ShiftInterruptDuration = value;
					OnPropertyChanged(nameof(ShiftInterruptDuration));
				}
			}
		}

		public uint ShiftInterruptCurrentThresholdPercent
		{
			get { return _config.ShiftInterruptCurrentThresholdPercent; }
			set
			{
				if (_config.ShiftInterruptCurrentThresholdPercent != value)
				{
					_config.ShiftInterruptCurrentThresholdPercent = value;
					OnPropertyChanged(nameof(ShiftInterruptCurrentThresholdPercent));
				}
			}
		}

		public ValueItemViewModel<Configuration.WalkModeData> WalkModeDataDisplay
		{
			get
			{
				return WalkModeDataDisplayOptions.FirstOrDefault((e) => e.Value == _config.WalkModeDataDisplay);
			}
			set
			{
				if (_config.WalkModeDataDisplay != value.Value)
				{
					_config.WalkModeDataDisplay = value.Value;
					OnPropertyChanged(nameof(WalkModeDataDisplay));
				}
			}
		}


		public uint StartupAssistLevel
		{
			get { return _config.AssistStartupLevel; }
			set
			{
				if (_config.AssistStartupLevel != value)
				{
					_config.AssistStartupLevel = value;
					OnPropertyChanged(nameof(StartupAssistLevel));
				}
			}
		}

		public ValueItemViewModel<Configuration.AssistModeSelect> AssistModeSelection
		{
			get
			{
				return AssistModeSelectOptions.FirstOrDefault((e) => e.Value ==  _config.AssistModeSelection);
			}
			set
			{
				if (_config.AssistModeSelection != value.Value)
				{
					_config.AssistModeSelection = value.Value;
					OnPropertyChanged(nameof(AssistModeSelection));
				}
			}
		}


		private List<AssistLevelViewModel> _standardAssistLevels;
		public List<AssistLevelViewModel> StandardAssistLevels
		{
			get { return _standardAssistLevels; }
			private set
			{
				if (_standardAssistLevels != value)
				{
					_standardAssistLevels = value;
					OnPropertyChanged(nameof(StandardAssistLevels));
				}
			}
		}

		private List<AssistLevelViewModel> _sportAssistLevels;
		public List<AssistLevelViewModel> SportAssistLevels
		{
			get { return _sportAssistLevels; }
			private set
			{
				if (_sportAssistLevels != value)
				{
					_sportAssistLevels = value;
					OnPropertyChanged(nameof(SportAssistLevels));
				}
			}
		}


		public ConfigurationViewModel()
		{
			_config = new Configuration(BbsfwConnection.Controller.Unknown);

			StandardAssistLevels = new List<AssistLevelViewModel>();
			SportAssistLevels = new List<AssistLevelViewModel>();
			
			for (int i = 0; i < _config.StandardAssistLevels.Length; ++i)
			{
				_standardAssistLevels.Add(new AssistLevelViewModel(this, i, _config.StandardAssistLevels[i]));
			}

			for (int i = 0; i < _config.SportAssistLevels.Length; ++i)
			{
				_sportAssistLevels.Add(new AssistLevelViewModel(this, i, _config.SportAssistLevels[i]));
			}
		}



		public void ReadConfiguration(string filepath)
		{
			_config.ReadFromFile(filepath);
			TriggerPropertyChanges();
		}

		public void WriteConfiguration(string filepath)
		{
			_config.WriteToFile(filepath);
		}

		public void UpdateFrom(Configuration config)
		{
			_config.CopyFrom(config);
			TriggerPropertyChanges();
		}

		public Configuration GetConfig()
		{
			return _config;
		}


		private static uint KphToMph(uint kph)
		{
			return (uint)Math.Round(kph * 0.621371192);
		}

		private static uint MphToKph(uint mph)
		{
			return (uint)Math.Round(mph * 1.609344);
		}

		private void TriggerPropertyChanges()
		{
			foreach (var prop in typeof(ConfigurationViewModel).GetProperties())
			{
				if (prop.GetGetMethod(false) != null)
				{
					OnPropertyChanged(prop.Name);
				}
			}

			// force update by creating new list
			StandardAssistLevels = StandardAssistLevels.ToList();
			SportAssistLevels = SportAssistLevels.ToList();
		}


	}
}
