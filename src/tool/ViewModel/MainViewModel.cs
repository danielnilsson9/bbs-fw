using BBSFW.Model;
using BBSFW.ViewModel.Base;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Input;

namespace BBSFW.ViewModel
{

	public class MainViewModel : ObservableObject
	{

		public ConfigurationViewModel ConfigVm { get; private set; }

		public ConnectionViewModel ConnectionVm { get; private set; }

		public SystemViewModel SystemVm { get; private set; }

		public AssistLevelsViewModel AssistLevelsVm { get; private set; }

		public EventLogViewModel EventLogVm { get; private set; }



		public ICommand OpenConfigCommand
		{
			get { return new DelegateCommand(OnOpenConfig); }
		}

		public ICommand SaveConfigCommand
		{
			get { return new DelegateCommand(OnSaveConfig); }
		}

		public ICommand ReadFlashCommand
		{
			get { return new DelegateCommand(OnReadFlash); }
		}

		public ICommand WriteFlashCommand
		{
			get { return new DelegateCommand(OnWriteFlash); }
		}

		public ICommand ExitCommand
		{
			get { return new DelegateCommand(OnExit); }
		}

		public ICommand ShowAboutCommand
		{
			get { return new DelegateCommand(OnShowAbout); }
		}



		public MainViewModel()
		{
			ConfigVm = new ConfigurationViewModel();


			ConnectionVm = new ConnectionViewModel();
			SystemVm = new SystemViewModel(ConfigVm);
			AssistLevelsVm = new AssistLevelsViewModel(ConfigVm);
			EventLogVm = new EventLogViewModel();
		}



		private void OnOpenConfig()
		{
			var dialog = new OpenFileDialog();
			dialog.Filter = "XML File|*.xml";
			dialog.Title = "Open Configuration";

			var result = dialog.ShowDialog();
			if (result.HasValue && result.Value)
			{
				ConfigVm.ReadConfiguration(dialog.FileName);
			}
		}

		private void OnSaveConfig()
		{
			var dialog = new SaveFileDialog();

			dialog.Filter = "XML File|*.xml";
			dialog.Title = "Save Configuration";
			dialog.FileName = "bbsfw.xml";

			var result = dialog.ShowDialog();
			if (result.HasValue && result.Value)
			{
				ConfigVm.WriteConfiguration(dialog.FileName);
			}
		}

		private async void OnReadFlash()
		{
			if (!ConnectionVm.IsConnected)
			{
				return;
			}

			VerifyConfigVersion();

			var res = await ConnectionVm.GetConnection().ReadConfiguration(TimeSpan.FromSeconds(5));
			if (!res.Timeout && res.Result != null)
			{
				ConfigVm.UpdateFrom(res.Result);
			}
			else
			{
				MessageBox.Show("Failed to read configuration from flash, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private async void OnWriteFlash()
		{
			if (!ConnectionVm.IsConnected)
			{
				return;
			}

			VerifyConfigVersion();

			var res = await ConnectionVm.GetConnection().WriteConfiguration(ConfigVm.GetConfig(), TimeSpan.FromSeconds(5));
			if (!res.Timeout)
			{
				if (res.Result)
				{
					MessageBox.Show("Configuration Written!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
				}
				else
				{
					MessageBox.Show("Failed to write configuration to flash, try again.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
			else
			{
				MessageBox.Show("Failed to write configuration to flash, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}



		private void OnShowAbout()
		{
			MessageBox.Show("Version: 1.0\nAuthor: Daniel Nilsson", "BBS-FW Tool", MessageBoxButton.OK, MessageBoxImage.Information);
		}

		private void OnExit()
		{
			Application.Current.Shutdown();
		}


		private bool VerifyConfigVersion()
		{
			if (ConnectionVm.ConfigVersion != Configuration.Version)
			{
				MessageBox.Show("Wrong firmware config version. Make sure you are using BBS-FW Config Tool for firmware version " + ConnectionVm.FirmwareVersion + ".", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return false;
			}

			return true;
		}

	}
}
