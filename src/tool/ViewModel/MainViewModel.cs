using BBSFW.Model;
using BBSFW.ViewModel.Base;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Windows;
using System.Windows.Input;

namespace BBSFW.ViewModel
{

	public class MainViewModel : ObservableObject
	{
		private const string APP_TITLE = "BBS-FW Tool";
		// Stores the name of the open config file. It's explicitly not a file handle, just a filename.
		private string _configFileName;

		// Has the configuration been modified by a reload or UI changes?
		private bool _configModified;
		public bool ConfigModified
		{
			get { return _configModified; }
			set
			{
				if (value != _configModified)
				{
					_configModified = value;
					OnPropertyChanged(nameof(ConfigModified));
				}
			}
		}

		// Stores a set of the changed UI properties so we can mark the config unmodified if it's reverted
		private HashSet<string> ChangedProperties = new HashSet<string>();

		// Store a set of properties to ignore because they're triggered by a related metric update
		private HashSet<string> ImperialProperties = new HashSet<string> {
			"PretensionSpeedCutoffMph", "MaxSpeedMph" };

		public string ConfigFileName
		{
			get { return _configFileName; }
			set
			{
				if (_configFileName != value)
				{
					_configFileName = value;
					OnPropertyChanged(nameof(ConfigFileName));
					OnPropertyChanged(nameof(ConfigFilenameExists));
				}
			}
		}

		/// <summary>
		/// Not sure if there's a better way of doing this, But it's used to hide certain menus when there is no file loaded.
		/// This check is trivial but allows us to do IsEnabled="{Binding ConfigFilenameExists}" in MainViewModel.xaml to show/hide menus.
		/// </summary>
		public bool ConfigFilenameExists
		{
			get
			{
				return !String.IsNullOrEmpty(ConfigFileName);
			}
		}

		private string _applicationTitle = "BBS - FW Tool";

		public string ApplicationTitle {
			get { return _applicationTitle; }
			set {
				if (_applicationTitle != value)
				{
					_applicationTitle = value;
					OnPropertyChanged(nameof(ApplicationTitle));
				}
			}
		}

		private string GetAppTitle()
		{
			string modified = ConfigModified ? " (modified)" : String.Empty;
			if (ConfigFilenameExists)
			{
				string fileName;
				try
				{
					// this would likely only fail if the config file was deleted or moved.
					fileName = System.IO.Path.GetFileName(ConfigFileName);
				} catch
				{
					// and if that's the case it's still valid to use that filename, we'll just make it explicit what the path is also.
					fileName = ConfigFileName;
				}

				return $"{APP_TITLE} - {fileName}{modified}";
			}
			else
			{
				return $"{APP_TITLE}{modified}";
			}
		}

		/// <summary>
		/// Original state of the configuration, so we can check for modifications
		/// </summary>
		private Configuration OriginalConfigurationState
		{
			get; set;
		}

		public ConfigurationViewModel ConfigVm { get; private set; }

		public ConnectionViewModel ConnectionVm { get; private set; }

		public SystemViewModel SystemVm { get; private set; }

		public AssistLevelsViewModel AssistLevelsVm { get; private set; }

		public CalibrationViewModel CalibrationVm { get; private set; }

		public EventLogViewModel EventLogVm { get; private set; }



		public ICommand OpenConfigCommand
		{
			get { return new DelegateCommand(OnOpenConfig); }
		}

		public ICommand OpenConfigDirectCommand
		{
			get { return new DelegateCommand(OnOpenConfigDirect); }
		}

		/**
		 * This doesnt actually close the config, it just clears the saved file name so you can't accidentally overwrite
		 */
		public ICommand CloseConfigCommand
		{
			get { return new DelegateCommand(OnCloseConfig); }
		}

		public ICommand SaveAsConfigCommand
		{
			get { return new DelegateCommand(OnSaveAsConfig); }
		}

		public ICommand SaveConfigCommand
		{
			get { return new DelegateCommand(OnSaveConfig); }
		}

		public ICommand SaveLogCommand
		{
			get { return new DelegateCommand(OnSaveLog); }
		}

		public ICommand ReadFlashCommand
		{
			get { return new DelegateCommand(OnReadFlash); }
		}

		public ICommand WriteFlashCommand
		{
			get { return new DelegateCommand(OnWriteFlash); }
		}

		public ICommand ResetFlashCommand
		{
			get { return new DelegateCommand(OnResetFlash); }
		}

		public ICommand ExitCommand
		{
			get { return new DelegateCommand(OnExit); }
		}

		public ICommand ShowAboutCommand
		{
			get { return new DelegateCommand(OnShowAbout); }
		}

		public ICommand UseMetricUnitsCommand
		{
			get { return new DelegateCommand(OnUseMetric); }
		}

		public ICommand UseImperialUnitsCommand
		{
			get { return new DelegateCommand(OnUseImperial); }
		}

		public MainViewModel()
		{
			ConfigVm = new ConfigurationViewModel();

			this.PropertyChanged += (sender, args) =>
			{
				// Updates the title when the config is modified (to mark it so), and the filename when it's set.
				if (args.PropertyName == nameof(ConfigModified) ||
					args.PropertyName == nameof(ConfigFileName))
				{
					ApplicationTitle = GetAppTitle();
				}
			};

			ConnectionVm = new ConnectionViewModel();
			SystemVm = new SystemViewModel(ConfigVm);
			AssistLevelsVm = new AssistLevelsViewModel(ConfigVm);
			CalibrationVm = new CalibrationViewModel(ConnectionVm);
			EventLogVm = new EventLogViewModel();
			if (!String.IsNullOrEmpty(Properties.Settings.Default.LastLoadedFile))
			{
				ConfigFileName = Properties.Settings.Default.LastLoadedFile;
				OpenConfigDirectCommand.Execute(ConfigFileName);
			}
			// start monitoring after load so we don't trigger all the events
			StartConfigMonitoring();
			ConnectionVm.EventLogReceived += EventLogVm.AddEvent;
		}

		/// <summary>
		/// Updates the OriginalConfigurationState to match the current config, for when loading/reseting etc.
		/// </summary>
		private void ResetOriginalConfigurationState()
		{
			OriginalConfigurationState = ConfigVm.GetConfig().Clone();
		}

		/// <summary>
		/// Start monitoring the ConfigurationViewModel for changes so we can mark it as modified when the user changes something in the UI.
		/// </summary>
		private void ConfigMonitoring(object? sender, PropertyChangedEventArgs args)
		{
			var propertyName = args.PropertyName;
			if (propertyName != null && sender != null && !ImperialProperties.Contains(propertyName))
			{
				var property = sender.GetType().GetProperty(propertyName);
				if (property != null)
				{
					var value = property.GetValue(sender) ?? "NULL";
					string originalValue = OriginalConfigurationState.GetType().GetField(propertyName)?.GetValue(OriginalConfigurationState).ToString() ?? "NULL";
					string newValue = value?.ToString();

					if (newValue != originalValue)
					{
						ConfigModified = true;
						ChangedProperties.Add(propertyName);
					}
					else
					{
						ChangedProperties.Remove(propertyName);
						// no values left means we have the original config again
						if (ChangedProperties.Count == 0)
						{
							ConfigModified = false;
						}
					}
				}
			}
		}

		private void OnSaveLog()
		{
			var dialog = new SaveFileDialog();

			dialog.Filter = "Log File|*.log";
			dialog.Title = "Save Log";
			dialog.FileName = "bbsfw.log";

			var result = dialog.ShowDialog();
			if (result.HasValue && result.Value)
			{
				try
				{
					EventLogVm.ExportLog(dialog.FileName);
				}
				catch (Exception e)
				{
					MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
		}

		private void OnCloseConfig()
		{
			ConfigFileName = null;
			ConfigModified = false;
		}

		private void StopConfigMonitoring()
		{
			ChangedProperties.Clear();
			ConfigVm.PropertyChanged -= ConfigMonitoring;
		}
		private void StartConfigMonitoring()
		{
			ChangedProperties.Clear();
			ResetOriginalConfigurationState();
			ConfigVm.PropertyChanged += ConfigMonitoring;
		}

		private void OnOpenConfig()
		{
			var dialog = new OpenFileDialog();
			dialog.Filter = "XML File|*.xml";
			dialog.Title = "Open Configuration";

			var result = dialog.ShowDialog();
			if (result.HasValue && result.Value)
			{
				try
				{
					StopConfigMonitoring();
					ConfigVm.ReadConfiguration(dialog.FileName);
					ConfigFileName = dialog.FileName;
					ConfigModified = false;
				}
				catch (Exception e)
				{
					MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
				finally
				{
					StartConfigMonitoring();
				}
			}
		}
		private void OnOpenConfigDirect()
		{
			try
			{
				StopConfigMonitoring();
				ConfigVm.ReadConfiguration(_configFileName);
				ConfigFileName = _configFileName;
				ConfigModified = false;
				StartConfigMonitoring();
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void OnSaveConfig()
		{
			if (!ValidateConfig())
			{
				return;
			}

			if (!ConfigFilenameExists)
			{
				OnSaveAsConfig();
				return;
			}

			try
			{
				ConfigVm.WriteConfiguration(ConfigFileName);
				ConfigModified = false;
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void OnSaveAsConfig()
		{
			if (!ValidateConfig())
			{
				return;
			}

			var dialog = new SaveFileDialog();

			dialog.Filter = "XML File|*.xml";
			dialog.Title = "Save Configuration";
			if (String.IsNullOrEmpty(ConfigFileName))
			{
				dialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
				dialog.FileName = "bbsfw.xml";
			}
			else
			{
				dialog.InitialDirectory = System.IO.Path.GetDirectoryName(ConfigFileName);
				dialog.FileName = System.IO.Path.GetFileName(ConfigFileName);
			}

			var result = dialog.ShowDialog();
			if (result.HasValue && result.Value)
			{
				try
				{
					ConfigVm.WriteConfiguration(dialog.FileName);
					// Updating the config file name will also update the application title.
					ConfigFileName = dialog.FileName;
					ConfigModified = false;
				}
				catch (Exception e)
				{
					MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
		}

		private async void OnReadFlash()
		{
			if (!ConnectionVm.IsConnected)
			{
				return;
			}

			if (!VerifyConfigVersionForRead())
			{
				return;
			}

			var res = await ConnectionVm.GetConnection().ReadConfiguration(TimeSpan.FromSeconds(5));
			if (!res.Timeout && res.Result != null)
			{
				StopConfigMonitoring();
				ConfigVm.UpdateFrom(res.Result);
				StartConfigMonitoring();
			}
			else
			{
				MessageBox.Show("Failed to read configuration from flash, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
			ConfigModified = true;
		}

		private async void OnWriteFlash()
		{
			if (!ConnectionVm.IsConnected)
			{
				return;
			}

			if (!ValidateConfig())
			{
				return;
			}

			if (!VerifyConfigVersionForWrite())
			{
				return;
			}

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

		private async void OnResetFlash()
		{
			if (!ConnectionVm.IsConnected)
			{
				return;
			}

			var res = await ConnectionVm.GetConnection().ResetConfiguration(TimeSpan.FromSeconds(5));
			if (!res.Timeout)
			{
				if (res.Result)
				{
					OnReadFlash();
				}
				else
				{
					MessageBox.Show("Failed to reset configuration, try again.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
			else
			{
				MessageBox.Show("Failed to reset configuration, timeout occured.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void OnUseMetric()
		{
			ConfigVm.UseMetricUnits = true;
		}

		private void OnUseImperial()
		{
			ConfigVm.UseImperialUnits = true;
		}

		private void OnShowAbout()
		{
			var version = Assembly.GetExecutingAssembly().GetName().Version;
			MessageBox.Show($"Version: {version.Major}.{version.Minor}.{version.Build}\nAuthor: Daniel Nilsson", "BBS-FW Tool", MessageBoxButton.OK, MessageBoxImage.Information);
		}

		/// <summary>
		/// Displays the save/save as dialog as necessary. This should probably be in a service class?
		/// </summary>
		/// <returns>True if this exit was cancelled</returns>
		public bool OnExitCancellable()
		{
			if (ConfigModified)
			{
				var result = MessageBox.Show($"You have unsaved changes, would you like to save them?",
					"BBS-FW Tool", MessageBoxButton.YesNoCancel, MessageBoxImage.Information);

				if (result == MessageBoxResult.Yes)
				{
					if (ConfigFilenameExists)
					{
						OnSaveConfig();
					}
					else
					{
						OnSaveAsConfig();
					}
				}
				return (result == MessageBoxResult.Cancel);
			}
			else
			{
				Properties.Settings.Default.LastLoadedFile = ConfigFileName ?? "";
				Properties.Settings.Default.Save();
				return false;
			}
		}

		private void OnExit()
		{
			
			Application.Current.MainWindow.Close();
		}

		private bool VerifyConfigVersionForRead()
		{
			if (ConnectionVm.ConfigVersion < Configuration.MinVersion || ConnectionVm.ConfigVersion > Configuration.MaxVersion)
			{
				MessageBox.Show("Unsupported firmware config version. Please use BBS-FW Config Tool for firmware version " + ConnectionVm.FirmwareVersion + " to read configuration from flash.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return false;
			}

			return true;
		}

		private bool VerifyConfigVersionForWrite()
		{
			if (ConnectionVm.ConfigVersion != Configuration.CurrentVersion)
			{
				MessageBox.Show("Unsupported firmware config version. Please use BBS-FW Config Tool for firmware version " + ConnectionVm.FirmwareVersion + " in order to write configuration to flash, or upgrade firmware to latest version.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
				return false;
			}

			return true;
		}

		private bool ValidateConfig()
		{
			try
			{
				ConfigVm.GetConfig().Validate();
				return true;
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, "Validation Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}

			return false;
		}

	}
}
