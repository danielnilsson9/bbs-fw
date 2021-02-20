using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Text;

namespace BBSFW.ViewModel
{
	public class ConnectionViewModel : ObservableObject
	{

		private List<ComPort> _comPorts;
		public List<ComPort> ComPorts
		{
			get { return _comPorts; }
			set
			{
				if (_comPorts != value)
				{
					_comPorts = value;
					OnPropertyChanged(nameof(ComPorts));
				}
			}
		}

		private ComPort _selectedComPort;
		public ComPort SelectedComPort
		{
			get { return _selectedComPort; }
			set
			{
				if (_selectedComPort != value)
				{
					_selectedComPort = value;
					OnPropertyChanged(nameof(SelectedComPort));
				}
			}
		}



		public ConnectionViewModel()
		{
			ComPorts = BbsfwConnection.GetComPorts();
		}




	}
}
