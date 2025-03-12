using System;
using System.Diagnostics.CodeAnalysis;

namespace BBSFW.ViewModel
{
	public class ValueItemViewModel<T> : IEquatable<ValueItemViewModel<T>>
	{
		public T Value { get; private set; }


		public string Name { get; private set; }


		public ValueItemViewModel(T value, string name)
		{
			Value = value;
			Name = name;
		}

		public static implicit operator T(ValueItemViewModel<T> v) => v.Value; 


		public override string ToString()
		{
			return Name;
		}

		public bool Equals([AllowNull] ValueItemViewModel<T> other)
		{
			return Value.Equals(other.Value);
		}
	}
}
