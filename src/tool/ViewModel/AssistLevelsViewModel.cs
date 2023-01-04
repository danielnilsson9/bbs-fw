using BBSFW.ViewModel.Base;
using System.Collections.Generic;


namespace BBSFW.ViewModel
{
	public class AssistLevelsViewModel : ObservableObject
	{

		public enum OperationMode
		{
			Standard,
			Sport
		}


		public static List<ValueItemViewModel<OperationMode>> OperationModes { get; } =
			new List<ValueItemViewModel<OperationMode>>
			{
				new ValueItemViewModel<OperationMode>(OperationMode.Standard, "Standard"),
				new ValueItemViewModel<OperationMode>(OperationMode.Sport, "Sport")
			};


		private ConfigurationViewModel _configVm;
		public ConfigurationViewModel ConfigVm
		{
			get { return _configVm; }
		}


		private ValueItemViewModel<OperationMode> _selectedOperationModePage;
		public ValueItemViewModel<OperationMode> SelectedOperationModePage
		{
			get { return _selectedOperationModePage; }
			set
			{
				if (_selectedOperationModePage != value)
				{
					_selectedOperationModePage = value;
					OnPropertyChanged(nameof(SelectedOperationModePage));
				}
			}
		}

		private AssistLevelViewModel _selectedAssistLevel;
		public AssistLevelViewModel SelectedAssistLevel
		{
			get { return _selectedAssistLevel; }
			set
			{
				if (_selectedAssistLevel != value)
				{
					_selectedAssistLevel = value;
					OnPropertyChanged(nameof(SelectedAssistLevel));
				}
			}
		}

		public AssistLevelsViewModel(ConfigurationViewModel config)
		{
			_configVm = config;
			SelectedOperationModePage = OperationModes[0];
		}

	}
}
