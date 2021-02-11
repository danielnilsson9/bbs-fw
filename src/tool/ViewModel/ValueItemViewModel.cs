namespace BBSFW.ViewModel
{
	public class ValueItemViewModel<T>
	{
		public T Value { get; private set; }


		public string Name { get; private set; }


		public ValueItemViewModel(T value, string name)
		{
			Value = value;
			Name = name;
		}

		public override string ToString()
		{
			return Name;
		}

	}
}
