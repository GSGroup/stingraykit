#ifndef STINGRAYKIT_IO_BITHREADPIPE_H
#define STINGRAYKIT_IO_BITHREADPIPE_H

#include <stingraykit/io/IPipe.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/time/ElapsedTime.h>

namespace stingray
{

	class BithreadPipe : public virtual IPipe
	{
	private:
		Mutex					_guard;

		ConditionVariable		_full;
		ConditionVariable		_empty;

		optional<ConstByteData>	_data;

	public:
		virtual u64 Read(ByteData data, const ICancellationToken& token = DummyCancellationToken(), const optional<TimeDuration>& timeout = null)
		{
			MutexLock l(_guard);

			ElapsedTime et;
			while (!_data && token)
			{
				if (!timeout)
					_full.Wait(_guard, token);
				else
					STINGRAYKIT_CHECK(_full.TimedWait(_guard, *timeout - et.Elapsed(), token) || _data, TimeoutException());
			}
			STINGRAYKIT_CHECK(token, OperationCancelledException());

			const size_t size = std::min(data.size(), _data->size());
			std::copy(_data->begin(), _data->begin() + size, data.data());

			_data = size == _data->size() ? null : optional<ConstByteData>(ConstByteData(*_data, size));
			if (!_data)
				_empty.Broadcast();

			return size;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token = DummyCancellationToken(), const optional<TimeDuration>& timeout = null)
		{
			MutexLock l(_guard);

			_data = data;
			_full.Broadcast();

			try
			{
				ElapsedTime et;
				while (_data && token)
				{
					if (!timeout)
						_empty.Wait(_guard, token);
					else
						STINGRAYKIT_CHECK(_empty.TimedWait(_guard, *timeout - et.Elapsed(), token) || !_data, TimeoutException());
				}
				STINGRAYKIT_CHECK(token, OperationCancelledException());
			}
			catch (const std::exception&)
			{
				_data = null;
				throw;
			}

			return data.size();
		}
	};

}

#endif
