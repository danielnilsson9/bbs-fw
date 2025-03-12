using System;
using System.Threading;
using System.Threading.Tasks;

namespace BBSFW.Model
{

	public class RequestResult<T>
	{
		public bool Timeout { get; private set; }

		public T Result { get; private set; }


		public RequestResult(bool timeout, T result)
		{
			Timeout = timeout;
			Result = result;
		}
	}


	public class CompletionQueue<T>
	{
		private TaskCompletionSource<T> _tcs = null;

		public void Complete(T response)
		{
			_tcs?.SetResult(response);
		}

		public async Task<RequestResult<T>> WaitResponse(TimeSpan timeout)
		{
			Reset(timeout);

			try
			{
				var res =  await _tcs.Task;
				_tcs = null;
				return new RequestResult<T>(false, res);
			}
			catch(TaskCanceledException)
			{
				_tcs = null;
				return new RequestResult<T>(true, default(T));
			}
		}

		private void Reset(TimeSpan timeout)
		{
			var tcs = new TaskCompletionSource<T>();

			var cancelTokenSrc = new CancellationTokenSource((int)timeout.TotalMilliseconds);
			cancelTokenSrc.Token.Register(() => tcs.TrySetCanceled());

			_tcs = tcs;
		}

	}
}
