using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Text;
using System.Threading;

namespace BBSFW.Model
{
	public class ComPort
	{
		public string Name { get; private set; }

		public string Description { get; private set; }


		public ComPort(string name, string description)
		{
			Name = name;
			Description = description;
		}
	}


	public class BbsfwConnection
	{
		private SerialPort _port = null;
		private Thread _comThread;



		public bool IsConnected { get; private set; }



		public event Action Connected;
		public event Action Disconnected;


		public void Open(ComPort port)
		{
			_port = new SerialPort(port.Name, 1200);
			_port.Open();

			// :TODO: start sending hello until response

			_comThread = new Thread(ComThreadRun);
			_comThread.Start();

		}

		public void Close()
		{
			if (_port != null)
			{
				_port.Close();
				_port = null;
			}
		}



		public static List<ComPort> GetComPorts()
		{
			var result = new List<ComPort>();

			using (var searcher = new ManagementObjectSearcher("SELECT * FROM WIN32_SerialPort"))
			{
				var portNames = SerialPort.GetPortNames();
				var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();

				foreach (var name in portNames)
				{
					var port = ports.FirstOrDefault(p => String.Equals(p["DeviceID"].ToString(), name));
					if (port != null)
					{
						result.Add(new ComPort(name, port["Caption"].ToString()));
					}
				}
			}

			return result;
		}





		private void ComThreadRun()
		{

			while (true)
			{

			}



		}


	}
}
