using System;
using System.Globalization;
using System.Windows.Data;

namespace BBSFW.View.Converter
{
	public class TimestampConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
		{
			if (value is DateTime)
			{
				var dt = (DateTime)value;

				return dt.ToString("yyyy-MM-dd HH:mm:ss.fff");
			}

			return null;
		}

		public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
		{
			throw new NotImplementedException();
		}
	}
}
