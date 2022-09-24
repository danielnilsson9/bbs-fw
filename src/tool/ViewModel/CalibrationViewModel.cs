using BBSFW.ViewModel.Base;
using System;
using System.Windows;
using System.Windows.Input;

namespace BBSFW.ViewModel
{
	public class CalibrationViewModel : ObservableObject
	{
		private ConnectionViewModel _connectionVm;

		private float _batteryStatusVolts;
		public float BatteryStatusVolts
		{
			get { return _batteryStatusVolts; }
			set
			{
				if (_batteryStatusVolts != value)
				{
					_batteryStatusVolts = value;
					OnPropertyChanged(nameof(BatteryStatusVolts));
				}
			}
		}

		private float _measuredBatteryVolts;
		public float MeasuredBatteryVolts
		{
			get { return _measuredBatteryVolts; }
			set
			{
				if (_measuredBatteryVolts != value)
				{
					_measuredBatteryVolts = value;
					OnPropertyChanged(nameof(MeasuredBatteryVolts));
				}
			}
		}


		public ICommand SaveVoltageCommand
		{
			get { return new DelegateCommand(OnSaveVoltageCalibration); }
		}

		public ICommand ResetVoltageCommand
		{
			get { return new DelegateCommand(OnResetVoltageCalibration); }
		}


		public CalibrationViewModel(ConnectionViewModel connectionVm)
		{
			_connectionVm = connectionVm;
		}


		private async void OnSaveVoltageCalibration()
		{
			if (!_connectionVm.IsConnected)
			{
				MessageBox.Show("Not Connected!", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return;
			}

			if (MeasuredBatteryVolts < 1 || MeasuredBatteryVolts > 100)
			{
				MessageBox.Show("Measured Battery Voltage must be in range [1, 100]", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return;
			}

			var res = await _connectionVm.GetConnection().CalibrateBatteryVoltage(MeasuredBatteryVolts, TimeSpan.FromSeconds(3));
			if (!res.Timeout)
			{
				if (res.Result)
				{
					MessageBox.Show("Voltage calibration saved!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
				}
				else
				{
					MessageBox.Show("Failed to save voltage calibration, check log.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
			else
			{
				MessageBox.Show("Failed to save voltage calibration, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private async void OnResetVoltageCalibration()
		{
			if (!_connectionVm.IsConnected)
			{
				MessageBox.Show("Not Connected!", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return;
			}

			var res = await _connectionVm.GetConnection().CalibrateBatteryVoltage(0f, TimeSpan.FromSeconds(3));
			if (!res.Timeout)
			{
				if (res.Result)
				{
					MessageBox.Show("Voltage calibration reset!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
				}
				else
				{
					MessageBox.Show("Failed to reset voltage calibration, check log.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
			else
			{
				MessageBox.Show("Failed to reset voltage calibration, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

	}
}
