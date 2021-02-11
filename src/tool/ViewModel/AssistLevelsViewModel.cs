using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BBSFW.ViewModel
{
	public class AssistLevelsViewModel : ObservableObject
	{


		public enum OperationModeSwitching
		{
			Off,
			SportButton,
			LightButton,
			Pas0PlusLightButton
		}

		public enum OperationMode
		{
			Default,
			Sport
		}

		public List<AssistLevelViewModel> AssistLevels { get; private set; }


		public List<ValueItemViewModel<OperationMode>> OperationModes { get; private set; }

		public List<ValueItemViewModel<OperationModeSwitching>> OperationModeSwitchingOptions { get; private set; }

		private ValueItemViewModel<OperationModeSwitching> _selectedOperationModeSwitching;
		public ValueItemViewModel<OperationModeSwitching> SelectedOperationModeSwitching
		{
			get { return _selectedOperationModeSwitching; }
			set
			{
				if (_selectedOperationModeSwitching != value)
				{
					_selectedOperationModeSwitching = value;
					OnPropertyChanged(nameof(SelectedOperationModeSwitching));
				}
			}
		}

		public List<int> SelectableDefaultAssistLevels { get; private set; }

		private int _selectedDefaultAssistLevel;
		public int SelectedDefaultAssistLevel
		{
			get { return _selectedDefaultAssistLevel; }
			set
			{
				if (_selectedDefaultAssistLevel != value)
				{
					_selectedDefaultAssistLevel = value;
					OnPropertyChanged(nameof(SelectedDefaultAssistLevel));
				}
			}
		}


		public AssistLevelsViewModel()
		{
			AssistLevels = new List<AssistLevelViewModel>();
			SelectableDefaultAssistLevels = new List<int>();
			for(int i = 0; i < 10; ++i)
			{
				SelectableDefaultAssistLevels.Add(i);
				AssistLevels.Add(new AssistLevelViewModel(i));
			}

			OperationModes = new List<ValueItemViewModel<OperationMode>>();
			OperationModes.Add(new ValueItemViewModel<OperationMode>(OperationMode.Default, "Default"));
			OperationModes.Add(new ValueItemViewModel<OperationMode>(OperationMode.Sport, "Sport"));

			OperationModeSwitchingOptions = new List<ValueItemViewModel<OperationModeSwitching>>();
			OperationModeSwitchingOptions.Add(new ValueItemViewModel<OperationModeSwitching>(OperationModeSwitching.Off, "Off"));
			OperationModeSwitchingOptions.Add(new ValueItemViewModel<OperationModeSwitching>(OperationModeSwitching.SportButton, "Sport Button"));
			OperationModeSwitchingOptions.Add(new ValueItemViewModel<OperationModeSwitching>(OperationModeSwitching.LightButton, "Lights Button"));
			OperationModeSwitchingOptions.Add(new ValueItemViewModel<OperationModeSwitching>(OperationModeSwitching.Pas0PlusLightButton, "PAS 0 + Lights Buttons"));

			SelectedOperationModeSwitching = OperationModeSwitchingOptions.First();

		}







	}
}
