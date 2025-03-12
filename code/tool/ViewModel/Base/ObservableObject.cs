using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;

namespace BBSFW.ViewModel.Base
{
	public class ObservableObject : INotifyPropertyChanged, INotifyDataErrorInfo
	{

		#region Private Members

		private Dictionary<string, string> _errors = new Dictionary<string, string>();

		#endregion

		#region Public Properties

		public bool HasErrors
		{
			get
			{
				return _errors.Count > 0;
			}
		}

		#endregion

		#region Public events

		public event EventHandler<DataErrorsChangedEventArgs> ErrorsChanged;
		public event PropertyChangedEventHandler PropertyChanged;

		#endregion

		#region Public Functions

		public void AddError(string propertyName, string error)
		{
			if (!_errors.ContainsKey(propertyName) || _errors[propertyName] != error)
			{
				_errors[propertyName] = error;
				ErrorsChanged?.Invoke(this, new DataErrorsChangedEventArgs(propertyName));
			}
		}

		public void RemoveError(string propertyName)
		{
			if (_errors.Remove(propertyName))
			{
				ErrorsChanged?.Invoke(this, new DataErrorsChangedEventArgs(propertyName));
			}
		}

		public string GetError(string propertyName)
		{
			if (_errors.ContainsKey(propertyName))
			{
				return _errors[propertyName];
			}

			return null;
		}

		public bool HasError(string propertyName)
		{
			return _errors.ContainsKey(propertyName);
		}

		public IEnumerable GetErrors(string propertyName)
		{
			if (propertyName == null)
				return null;

			List<string> err = new List<string>();
			if (_errors.ContainsKey(propertyName))
			{
				err.Add(_errors[propertyName]);
			}

			return err;
		}

		#endregion

		#region Protected Functions

		protected void OnPropertyChanged(string propertyName)
		{
			PropertyChanged?.Invoke(this,
				new PropertyChangedEventArgs(propertyName));
		}

		#endregion

	}
}
