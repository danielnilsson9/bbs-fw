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

		private void OnShowAbout()
		{
			MessageBox.Show("Version: 1.0\nAuthor: Daniel Nilsson", "BBS-FW Tool", MessageBoxButton.OK, MessageBoxImage.Information);
		}

		private void OnExit()
		{
			Application.Current.Shutdown();
		}

	}
}
