using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Windows;
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


		private BbsfwConnection.Controller _controller;
		public BbsfwConnection.Controller Controller
		{
			get { return _controller; }
			set
			{
				if (_controller != value)
				{
					_controller = value;
					OnPropertyChanged(nameof(Controller));
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
			private set
			{
				if (_firmwareVersion != value)
				{
					_firmwareVersion = value;
					OnPropertyChanged(nameof(FirmwareVersion));
				}
			}
		}

		private int _configVersion = 0;
		public int ConfigVersion
		{
			get
			{
				return _configVersion;
			}
			private set
			{
				if (_configVersion != value)
				{
					_configVersion = value;
					OnPropertyChanged(nameof(ConfigVersion));
				}
			}
		}


		public event Action<EventLogEntry> EventLogReceived;

		public ICommand RefreshCommand
		{
			get
			{
				return new DelegateCommand(OnRefresh);
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
			_connection.EventLog += (e) =>
			{
				EventLogReceived?.Invoke(e);
			};


			ComPorts = BbsfwConnection.GetComPorts();
		}


		public BbsfwConnection GetConnection()
		{
			return _connection;
		}


		private void OnConnected(BbsfwConnection.Controller controller, string fwversion, int configVersion)
		{
			IsConnected = true;
			IsConnecting = false;

			Controller = controller;
			FirmwareVersion = fwversion;
			ConfigVersion = configVersion;
		}

		private void OnDisconnected()
		{
			IsConnected = false;
			IsConnecting = false;
			FirmwareVersion = "N/A";
			ConfigVersion = 0;
		}

		private void OnRefresh()
		{
			ComPorts = BbsfwConnection.GetComPorts();
			OnPropertyChanged(nameof(ComPorts));
		}

		private async void OnConnect()
		{
			if (SelectedComPort != null)
			{
				IsConnecting = true;

				try
				{
					var connected = await _connection.Connect(SelectedComPort, TimeSpan.FromSeconds(120));

					if (!connected)
					{
						MessageBox.Show("Failed to connect, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
					}
				}
				catch (Exception ex)
				{
					MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
					IsConnected = false;
					IsConnecting = false;
				}	
			}
		}

		private void OnDisconnect()
		{
			_connection.Close();
		}

	}
}
