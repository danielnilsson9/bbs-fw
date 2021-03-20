using BBSFW.Model;
using BBSFW.ViewModel.Base;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;

namespace BBSFW.ViewModel
{
	public class EventLogViewModel : ObservableObject
	{

		private ObservableCollection<EventLogEntry> _events = new ObservableCollection<EventLogEntry>();
		public ObservableCollection<EventLogEntry> LogEvents
		{
			get { return _events; }
			set
			{
				if (_events != value)
				{
					_events = value;
					OnPropertyChanged(nameof(LogEvents));
				}
			}
		}

		private ICollectionView _filtedLogEvents;
		public ICollectionView FilteredLogEvents
		{
			get { return _filtedLogEvents; }
		}


		public IEnumerable<EventLogEntry.LogLevel> AvailableLogLevels
		{
			get { return new[] { EventLogEntry.LogLevel.Info, EventLogEntry.LogLevel.Warning, EventLogEntry.LogLevel.Error }; }
		}

		private EventLogEntry.LogLevel _selectedLogLevel;
		public EventLogEntry.LogLevel SelectedLogLevel
		{
			get { return _selectedLogLevel; }
			set
			{
				if (_selectedLogLevel != value)
				{
					_selectedLogLevel = value;
					OnPropertyChanged(nameof(SelectedLogLevel));

					FilteredLogEvents.Refresh();
				}
			}
		}

		private string _filterText;
		public string FilterText
		{
			get { return _filterText; }
			set
			{
				if (_filterText != value)
				{
					_filterText = value;
					OnPropertyChanged(nameof(FilterText));
					_filtedLogEvents.Refresh();
				}
			}
		}


		private bool _tailLog;
		public bool TailLog
		{
			get { return _tailLog; }
			set
			{
				if (_tailLog != value)
				{
					_tailLog = value;
					OnPropertyChanged(nameof(TailLog));
				}
			}
		}



		public ICommand ClearCommand
		{
			get { return new DelegateCommand(OnClear); }
		}


		public EventLogViewModel()
		{
			_filtedLogEvents = (CollectionView)CollectionViewSource.GetDefaultView(LogEvents);
			_filtedLogEvents.Filter += OnFilterTriggered;
		}



		public void AddEvent(EventLogEntry e)
		{
			Application.Current.Dispatcher.InvokeAsync(() => LogEvents.Add(e));
		}

		public void ExportLog(string filepath)
		{
			using (var writer = new StreamWriter(filepath))
			{
				foreach (var e in LogEvents)
				{
					writer.WriteLine($"{e.Timestamp.ToString("yyyy-MM-dd HH:mm:ss.fff")}    {e.Level}    {e.Message}");
				}
			}	
		}



		private bool OnFilterTriggered(object obj)
		{
			var e = obj as EventLogEntry;

			if (e != null)
			{
				if (e.Level >= SelectedLogLevel)
				{
					if (String.IsNullOrEmpty(FilterText))
					{
						return true;
					}

					if (e.Message.IndexOf(FilterText, StringComparison.OrdinalIgnoreCase) >= 0)
					{
						return true;
					}
				}
			}

			return false;
		}


		private void OnClear()
		{
			LogEvents.Clear();
		}
	}
}
