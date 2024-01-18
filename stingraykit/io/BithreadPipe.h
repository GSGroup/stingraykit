// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_BITHREADPIPE_H
#define STINGRAYKIT_IO_BITHREADPIPE_H

#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>
#include <stingraykit/io/IPipe.h>
#include <stingraykit/thread/ConditionVariable.h>
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
		virtual u64 Read(ByteData data, const ICancellationToken& token = DummyCancellationToken())
		{
			MutexLock l(_guard);

			while (!_data)
				switch (_full.Wait(_guard, token))
				{
				case ConditionWaitResult::Broadcasted:	continue;
				case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
				case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
				}

			const size_t size = std::min(data.size(), _data->size());
			std::copy(_data->begin(), _data->begin() + size, data.data());

			_data = size == _data->size() ? null : make_optional_value<ConstByteData>(*_data, size);
			if (!_data)
				_empty.Broadcast();

			return size;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token = DummyCancellationToken())
		{
			MutexLock l(_guard);

			_data = data;
			_full.Broadcast();

			ScopeExitInvoker sei(Bind(make_assigner(_data), null));

			while (_data)
				switch (_empty.Wait(_guard, token))
				{
				case ConditionWaitResult::Broadcasted:	continue;
				case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
				case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
				}

			sei.Cancel();

			return data.size();
		}

		virtual bool Peek(const ICancellationToken& token = DummyCancellationToken())
		{
			MutexLock l(_guard);
			return _data.is_initialized();
		}
	};

}

#endif
