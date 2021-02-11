using BBSFW.ViewModel.Base;


namespace BBSFW.ViewModel
{
	public class AssistLevelViewModel : ObservableObject
	{

		public enum AssistType
		{
			Disabled = 0x00,
			Pas = 0x01,
			Throttle = 0x02,
			PasAndThrottle = 0x03,
			Cruise = 0x04
		};


		private int _id;
		public int Id
		{
			get { return _id; }
		}



		private AssistType _type;
		public AssistType Type
		{
			get { return _type; }
			set
			{
				if (_type != value)
				{
					_type = value;
					OnPropertyChanged(nameof(Type));
				}
			}
		}

		private int _targetCurrentPercent;
		public int TargetCurrentPercent
		{
			get { return _targetCurrentPercent; }
			set
			{
				if (_targetCurrentPercent != value)
				{
					_targetCurrentPercent = value;
					OnPropertyChanged(nameof(TargetCurrentPercent));
				}
			}
		}

		private int _maxThrottlePercent;
		public int MaxThrottlePercent
		{
			get { return _maxThrottlePercent; }
			set
			{
				if (_maxThrottlePercent != value)
				{
					_maxThrottlePercent = value;
					OnPropertyChanged(nameof(MaxThrottlePercent));
				}
			}
		}

		private int _maxSpeedPercent;
		public int MaxSpeedPercent
		{
			get { return _maxThrottlePercent; }
			set
			{
				if (_maxSpeedPercent != value)
				{
					_maxSpeedPercent = value;
					OnPropertyChanged(nameof(MaxSpeedPercent));
				}
			}
		}


		public AssistLevelViewModel(int id)
		{
			_id = id;
		}


	}
}
