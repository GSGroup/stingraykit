#ifndef STINGRAYKIT_THREAD_ASYNCQUEUEPROCESSOR_H
#define STINGRAYKIT_THREAD_ASYNCQUEUEPROCESSOR_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/ObservableValue.h>
#include <stingraykit/ProgressValue.h>

#include <list>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template < typename ValueType_ >
	class AsyncQueueProcessor
	{
	public:
		typedef ValueType_							ValueType;
		typedef function<void (const ValueType&)>	FunctorType;

	private:
		typedef std::list<ValueType> Queue;

	private:
		FunctorType						_processor;

		Mutex							_mutex;
		Queue							_queue;
		ConditionVariable				_condition;

		ObservableValue<bool>			_idle;
		ObservableValue<ProgressValue>	_progress;

		ThreadPtr						_thread;

	public:
		AsyncQueueProcessor(const std::string& name, const FunctorType& processor)
			:	_processor(processor),
				_idle(true)
		{ _thread = make_shared_ptr<Thread>(name, Bind(&AsyncQueueProcessor::ThreadFunc, this, _1)); }

		void PushFront(const ValueType& value)
		{
			MutexLock l(_mutex);
			_queue.push_front(value);
			_condition.Broadcast();

			IncreaseProgress(0, 1);
		}

		void PushBack(const ValueType& value)
		{
			MutexLock l(_mutex);
			_queue.push_back(value);
			_condition.Broadcast();

			IncreaseProgress(0, 1);
		}

		signal_connector<void (bool)> OnIdle() const
		{ return _idle.OnChanged(); }

		signal_connector<void (const ProgressValue&)> OnProgress() const
		{ return _progress.OnChanged(); }

	private:
		void IncreaseProgress(s64 CurrentDelta, s64 totalDelta)
		{
			MutexLock l(_progress.GetSyncRoot());

			const ProgressValue progress = _progress.Get();
			_progress.Set(ProgressValue(progress.Current + CurrentDelta, progress.Total + totalDelta));
		}

		void ThreadFunc(const ICancellationToken& token)
		{
			MutexLock l(_mutex);

			while (token)
			{
				if (_queue.empty())
				{
					_idle.Set(true);
					_progress.Set(ProgressValue());

					_condition.Wait(_mutex, token);
					continue;
				}

				_idle.Set(false);

				const ValueType value = _queue.front();
				_queue.pop_front();

				MutexUnlock ul(l);
				STINGRAYKIT_TRY("Processor func exception", _processor(value));

				IncreaseProgress(1, 0);
			}
		}
	};

	/** @} */

}

#endif
