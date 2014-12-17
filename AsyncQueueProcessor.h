#ifndef STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H
#define STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H

#include <stingray/toolkit/log/Logger.h>
#include <stingray/threads/CancellationToken.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/function/function.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/ProgressValue.h>
#include <list>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template<typename ValueType_>
	class AsyncQueueProcessor
	{
	public:
		typedef ValueType_							ValueType;
		typedef function<void (const ValueType &)>	FunctorType;

	private:
		FunctorType			_processor;

		CancellationToken	_token;
		ConditionVariable	_condition;
		Mutex				_lock;

		typedef std::list<ValueType> Queue;
		Queue				_queue;

		bool				_idle;
		ProgressValue		_progress;

		ThreadPtr			_thread;

	public:
		signal<void(bool)>				OnIdle;
		signal<void(ProgressValue)>		OnProgress;

		AsyncQueueProcessor(const std::string &name, const FunctorType &processor):
			_processor(processor),
			_idle(true),
			OnIdle(bind(&AsyncQueueProcessor::OnIdlePopulator, this, _1)),
			OnProgress(bind(&AsyncQueueProcessor::OnProgressPopulator, this, _1))
		{ _thread = make_shared<Thread>(name, bind(&AsyncQueueProcessor::ThreadFunc, this, _1)); }

		~AsyncQueueProcessor()
		{
			_thread.reset();
		}

		void PushFront(const ValueType &value)
		{
			{
				MutexLock l(_lock);
				_queue.push_front(value);
				_condition.Broadcast();
			}
			{
				signal_locker l(OnProgress);
				++_progress.Total;
				OnProgress(_progress);
			}
		}

		void PushBack(const ValueType &value)
		{
			{
				MutexLock l(_lock);
				_queue.push_back(value);
				_condition.Broadcast();
			}
			{
				signal_locker l(OnProgress);
				++_progress.Total;
				OnProgress(_progress);
			}
		}

	private:
		void OnIdlePopulator(const function<void (bool)> & slot)
		{ slot(_idle); }

		void OnProgressPopulator(const function<void (ProgressValue)> & slot)
		{ slot(_progress); }

		void SetIdle(MutexLock &lock, bool idle)
		{
			MutexUnlock ll(lock);
			signal_locker l(OnIdle);
			if (idle != _idle)
			{
				_idle = idle;
				OnIdle(_idle);
			}
		}

		void ThreadFunc(const ICancellationToken& token)
		{
			MutexLock l(_lock);
			while(token)
			{
				if (_queue.empty())
				{
					SetIdle(l, true);
					_condition.Wait(_lock, token);
					if (!_queue.empty())
						SetIdle(l, false);
					continue;
				}
				ValueType value = _queue.front();
				_queue.pop_front();

				MutexUnlock ll(l);
				STINGRAY_TRY("exception in queue processor", _processor(value));
				{
					signal_locker l(OnProgress);
					++_progress.Current;
					OnProgress(_progress);
				}
			}
		}
	};

	/** @} */

}


#endif
