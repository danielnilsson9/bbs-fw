using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Text;

namespace BBSFW.ViewModel
{
	public class SystemViewModel : ObservableObject
	{


		private ConfigurationViewModel _configVm;
		public ConfigurationViewModel ConfigVm
		{
			get { return _configVm; }
		}

		public SystemViewModel(ConfigurationViewModel config)
		{
			_configVm = config;
		}

	}
}
