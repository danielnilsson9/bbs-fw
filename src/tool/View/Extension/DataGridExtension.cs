using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;

namespace BBSFW.View.Extension
{
	public static class DataGridExtension
	{

		public static readonly DependencyProperty AutoScrollToEndProperty = DependencyProperty.RegisterAttached(
			"AutoScrollToEnd", typeof(bool), typeof(DataGridExtension), new PropertyMetadata(default(bool), AutoScrollToEndChangedCallback));

		private static readonly Dictionary<DataGrid, NotifyCollectionChangedEventHandler> handlersDict = new Dictionary<DataGrid, NotifyCollectionChangedEventHandler>();

		private static void AutoScrollToEndChangedCallback(DependencyObject dependencyObject, DependencyPropertyChangedEventArgs args)
		{
			var dataGrid = dependencyObject as DataGrid;
			if (dataGrid == null)
			{
				throw new InvalidOperationException("Dependency object is not DataGrid.");
			}

			if ((bool)args.NewValue)
			{
				Subscribe(dataGrid);
				dataGrid.Unloaded += DataGridOnUnloaded;
				dataGrid.Loaded += DataGridOnLoaded;
			}
			else
			{
				Unsubscribe(dataGrid);
				dataGrid.Unloaded -= DataGridOnUnloaded;
				dataGrid.Loaded -= DataGridOnLoaded;
			}
		}

		private static void Subscribe(DataGrid dataGrid)
		{
			var handler = new NotifyCollectionChangedEventHandler((sender, eventArgs) => ScrollToEnd(dataGrid));
			handlersDict.Add(dataGrid, handler);
			((INotifyCollectionChanged)dataGrid.Items).CollectionChanged += handler;
			ScrollToEnd(dataGrid);
		}

		private static void Unsubscribe(DataGrid dataGrid)
		{
			NotifyCollectionChangedEventHandler handler;
			handlersDict.TryGetValue(dataGrid, out handler);
			if (handler == null)
			{
				return;
			}
			((INotifyCollectionChanged)dataGrid.Items).CollectionChanged -= handler;
			handlersDict.Remove(dataGrid);
		}

		private static void DataGridOnLoaded(object sender, RoutedEventArgs routedEventArgs)
		{
			var dataGrid = (DataGrid)sender;
			if (GetAutoScrollToEnd(dataGrid))
			{
				Subscribe(dataGrid);
			}
		}

		private static void DataGridOnUnloaded(object sender, RoutedEventArgs routedEventArgs)
		{
			var dataGrid = (DataGrid)sender;
			if (GetAutoScrollToEnd(dataGrid))
			{
				Unsubscribe(dataGrid);
			}
		}

		private static void ScrollToEnd(DataGrid datagrid)
		{
			if (datagrid.Items.Count == 0)
			{
				return;
			}
			datagrid.ScrollIntoView(datagrid.Items[datagrid.Items.Count - 1]);
		}

		public static void SetAutoScrollToEnd(DependencyObject element, bool value)
		{
			element.SetValue(AutoScrollToEndProperty, value);
		}

		public static bool GetAutoScrollToEnd(DependencyObject element)
		{
			return (bool)element.GetValue(AutoScrollToEndProperty);
		}

	}
}
