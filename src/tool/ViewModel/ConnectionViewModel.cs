using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace BBSFW.ViewModel
{
	public class ConnectionViewModel : ObservableObject
	{

		private BbsfwConnection _connection;

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

		private bool _isConnected;
		public bool IsConnected
		{
			get { return _isConnected; }
			set
			{
				if (_isConnected != value)
				{
					_isConnected = value;
					OnPropertyChanged(nameof(IsConnected));
					OnPropertyChanged(nameof(IsDisconnected));
				}
			}
		}

		public bool IsDisconnected
		{
			get
			{
				return !IsConnected;
			}
		}

		private bool _isConnecting;
		public bool IsConnecting
		{
			get { return _isConnecting; }
			set
			{
				if (_isConnecting != value)
				{
					_isConnecting = value;
					OnPropertyChanged(nameof(IsConnecting));
				}	 
			}
		}

		private string _firmwareVersion = "N/A";
		public string FirmwareVersion
		{
			get
			{
				return _firmwareVersion;
			}
			set
			{
				if (_firmwareVersion != value)
				{
					_firmwareVersion = value;
					OnPropertyChanged(nameof(FirmwareVersion));
				}
			}
		}


		public ICommand ConnectCommand
		{
			get
			{
				return new DelegateCommand(OnConnect);
			}
		}

		public ICommand DisconnectCommand
		{
			get
			{
				return new DelegateCommand(OnDisconnect);
			}
		}

		public ConnectionViewModel()
		{
			_connection = new BbsfwConnection();

			_connection.Connected += OnConnected;
			_connection.Disconnected += OnDisconnected;


			ComPorts = BbsfwConnection.GetComPorts();
		}




		private void OnConnected()
		{
			IsConnected = true;
			IsConnecting = false;
		}

		private void OnDisconnected()
		{
			IsConnected = false;
			IsConnecting = false;
			FirmwareVersion = "N/A";
		}


		private void OnConnect()
		{
			if (SelectedComPort != null)
			{
				IsConnecting = true;
				_connection.Open(SelectedComPort);
			}
		}

		private void OnDisconnect()
		{
			_connection.Close();
		}

	}
}
