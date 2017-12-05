#ifndef STINGRAYKIT_IO_BITHREADPIPE_H
#define STINGRAYKIT_IO_BITHREADPIPE_H

#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>
#include <stingraykit/io/IPipe.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/TimedCancellationToken.h>
#include <stingraykit/ScopeExit.h>

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

			const ICancellationToken& timedToken = timeout ? (const ICancellationToken&)TimedCancellationToken(token, *timeout) : token;

			while (!_data)
				switch (_full.Wait(_guard, timedToken))
				{
				case ConditionWaitResult::Broadcasted:	continue;
				case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
				case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
				}

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

			ScopeExitInvoker sei(bind(make_assigner(_data), null));

			const ICancellationToken& timedToken = timeout ? (const ICancellationToken&)TimedCancellationToken(token, *timeout) : token;

			while (_data)
				switch (_empty.Wait(_guard, timedToken))
				{
				case ConditionWaitResult::Broadcasted:	continue;
				case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
				case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
				}

			sei.Cancel();

			return data.size();
		}
	};

}

#endif
