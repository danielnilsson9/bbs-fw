using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BBSFW.ViewModel
{
	public class AssistLevelsViewModel : ObservableObject
	{


		public enum OperationMode
		{
			Default,
			Sport
		}


		public static List<ValueItemViewModel<OperationMode>> OperationModes { get; } =
			new List<ValueItemViewModel<OperationMode>>
			{
				new ValueItemViewModel<OperationMode>(OperationMode.Default, "Default"),
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
					LoadAssistLevels(_selectedOperationModePage.Value);
					OnPropertyChanged(nameof(SelectedOperationModePage));
				}
			}
		}

		private List<AssistLevelViewModel> _currentAssistLevels;
		public List<AssistLevelViewModel> CurrentAssistLevels
		{
			get { return _currentAssistLevels; }
			set
			{
				if (_currentAssistLevels != value)
				{
					_currentAssistLevels = value;
					OnPropertyChanged(nameof(CurrentAssistLevels));
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





		private void LoadAssistLevels(OperationMode mode)
		{
			if (mode == OperationMode.Default)
			{
				CurrentAssistLevels = ConfigVm.GetDefaultAssistLevels();
			}
			else if (mode == OperationMode.Sport)
			{
				CurrentAssistLevels = ConfigVm.GetSportAssistLevels();
			}
		}

	}
}
