using BBSFW.ViewModel.Base;
using System.Collections.Generic;
using System.Linq;

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
					UpdateSelectedAssistLevel();
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

		/// <summary>
		/// Updates the assist level manually so we can refresh it when we switch mode
		/// </summary>
		private void UpdateSelectedAssistLevel()
		{
			var newAssist = SelectedOperationModePage.Value == OperationMode.Sport ?
				ConfigVm.SportAssistLevels : ConfigVm.StandardAssistLevels;
			int index = SelectedAssistLevel?.Id ?? 0;
			SelectedAssistLevel = newAssist.ElementAt(index);
		}

		public AssistLevelsViewModel(ConfigurationViewModel config)
		{
			_configVm = config;
			SelectedOperationModePage = OperationModes[0];
			UpdateSelectedAssistLevel();
		}
	}
}
