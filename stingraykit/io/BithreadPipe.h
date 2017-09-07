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
		virtual u64 Read(ByteData data, const ICancellationToken& token = DummyCancellationToken(), const optional<TimeDuration>& timeout)
		{
			MutexLock l(_guard);

			ElapsedTime et;
			while (!_data && token && (!timeout || et.Elapsed() < *timeout))
			{
				if (timeout)
					_full.TimedWait(_guard, *timeout - et.Elapsed(), token);
				else
					_full.Wait(_guard, token);
			}
			STINGRAYKIT_CHECK(token, OperationCancelledException());
			STINGRAYKIT_CHECK(!timeout || et.Elapsed() < *timeout, TimeoutException());

			const size_t size = std::min(data.size(), _data->size());
			std::copy(_data->begin(), _data->begin() + size, data.data());

			_data = size == _data->size() ? null : optional<ConstByteData>(ConstByteData(*_data, size));
			if (!_data)
				_empty.Broadcast();

			return size;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token = DummyCancellationToken(), const optional<TimeDuration>& timeout)
		{
			MutexLock l(_guard);

			_data = data;
			_full.Broadcast();

			ElapsedTime et;
			while (_data && token && (!timeout || et.Elapsed() < *timeout))
			{
				if (timeout)
					_empty.TimedWait(_guard, *timeout - et.Elapsed(), token);
				else
					_empty.Wait(_guard, token);
			}
			STINGRAYKIT_CHECK(token, OperationCancelledException());
			STINGRAYKIT_CHECK(!timeout || et.Elapsed() < *timeout, TimeoutException());

			return data.size();
		}
	};

}

#endif
