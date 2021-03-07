using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Text;

namespace BBSFW.ViewModel
{

	public class MainViewModel : ObservableObject
	{

		public ConfigurationViewModel ConfigVm { get; private set; }

		public ConnectionViewModel ConnectionVm { get; private set; }

		public SystemViewModel SystemVm { get; private set; }

		public AssistLevelsViewModel AssistLevelsVm { get; private set; }

		public EventLogViewModel EventLogVm { get; private set; }


		public MainViewModel()
		{
			ConfigVm = new ConfigurationViewModel();


			ConnectionVm = new ConnectionViewModel();
			SystemVm = new SystemViewModel(ConfigVm);
			AssistLevelsVm = new AssistLevelsViewModel(ConfigVm);
			EventLogVm = new EventLogViewModel();
		}

	}
}
