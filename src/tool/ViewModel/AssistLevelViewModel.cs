using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System.Collections.Generic;
using System.Linq;

namespace BBSFW.ViewModel
{
	public class AssistLevelViewModel : ObservableObject
	{

		private Configuration.AssistLevel _level;


		public static List<ValueItemViewModel<Configuration.AssistType>> AssistTypeOptions { get; } =
			new List<ValueItemViewModel<Configuration.AssistType>>
			{
				new ValueItemViewModel<Configuration.AssistType>(Configuration.AssistType.Disabled, "Motor Disabled"),
				new ValueItemViewModel<Configuration.AssistType>(Configuration.AssistType.Pas, "PAS"),
				new ValueItemViewModel<Configuration.AssistType>(Configuration.AssistType.Throttle, "Throttle Only"),
				new ValueItemViewModel<Configuration.AssistType>(Configuration.AssistType.PasAndThrottle, "PAS & Throttle"),
				new ValueItemViewModel<Configuration.AssistType>(Configuration.AssistType.Cruise, "Cruise")
			};


		private int _id;
		public int Id
		{
			get { return _id; }
		}

		public ValueItemViewModel<Configuration.AssistType> Type
		{
			get { return AssistTypeOptions.FirstOrDefault((e) => e.Value == _level.Type); }
			set
			{
				if (_level.Type != value)
				{
					_level.Type = value;
					OnPropertyChanged(nameof(Type));

					switch (value.Value)
					{
						case Configuration.AssistType.Disabled:
							TargetCurrentPercent = 0;
							MaxThrottlePercent = 0;
							MaxSpeedPercent = 0;
							break;
						case Configuration.AssistType.Throttle:
							TargetCurrentPercent = 0;
							break;
						case Configuration.AssistType.Pas:
							MaxThrottlePercent = 0;
							break;
					}
				}
			}
		}

		public uint TargetCurrentPercent
		{
			get { return _level.MaxCurrentPercent; }
			set
			{
				if (_level.MaxCurrentPercent != value)
				{
					_level.MaxCurrentPercent = value;
					OnPropertyChanged(nameof(TargetCurrentPercent));
				}
			}
		}

		public uint MaxThrottlePercent
		{
			get { return _level.MaxThrottlePercent; }
			set
			{
				if (_level.MaxThrottlePercent != value)
				{
					_level.MaxThrottlePercent = value;
					OnPropertyChanged(nameof(MaxThrottlePercent));
				}
			}
		}

		public uint MaxCadencePercent
		{
			get { return _level.MaxCadencePercent; }
			set
			{
				if (_level.MaxCadencePercent != value)
				{
					_level.MaxCadencePercent = value;
					OnPropertyChanged(nameof(MaxCadencePercent));
				}
			}
		}

		public uint MaxSpeedPercent
		{
			get { return _level.MaxSpeedPercent; }
			set
			{
				if (_level.MaxSpeedPercent != value)
				{
					_level.MaxSpeedPercent = value;
					OnPropertyChanged(nameof(MaxSpeedPercent));
				}
			}
		}


		public AssistLevelViewModel(int id, Configuration.AssistLevel level)
		{
			_id = id;
			_level = level;
		}

	}
}
