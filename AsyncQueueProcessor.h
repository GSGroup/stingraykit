#ifndef STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H
#define STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H

#include <stingray/threads/ConditionVariable.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/function.h>
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
		bool				_running;
		FunctorType			_processor;

		ConditionVariable	_cond;
		Mutex				_lock;

		typedef std::list<ValueType> Queue;
		Queue				_queue;

		bool				_idle;
		ProgressValue		_progress;

	public:
		signal<void(bool)>				OnIdle;
		signal<void(ProgressValue)>		OnProgress;

		AsyncQueueProcessor(const std::string &name, const FunctorType &processor):
			_running(true),
			_processor(processor),
			_idle(true),
			OnIdle(bind(&AsyncQueueProcessor::OnIdlePopulator, this, _1)),
			OnProgress(bind(&AsyncQueueProcessor::OnProgressPopulator, this, _1)),
			_thread(name, bind(&AsyncQueueProcessor::ThreadFunc, this))
		{ }

		~AsyncQueueProcessor()
		{
			{
				MutexLock l(_lock);
				_running = false;
				_cond.Broadcast();
			}
			_thread.Interrupt();
			_thread.Join();
		}

		void PushFront(const ValueType &value)
		{
			{
				MutexLock l(_lock);
				_queue.push_front(value);
				_cond.Broadcast();
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
				_cond.Broadcast();
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

		void ThreadFunc()
		{
			MutexLock l(_lock);
			while(_running)
			{
				if (_queue.empty())
				{
					SetIdle(l, true);
					_cond.Wait(_lock);
					if (!_queue.empty())
						SetIdle(l, false);
					continue;
				}
				ValueType value = _queue.front();
				_queue.pop_front();

				MutexUnlock ll(l);
				STINGRAY_TRY_DO("exception in queue processor", _processor(value));
				{
					signal_locker l(OnProgress);
					++_progress.Current;
					OnProgress(_progress);
				}
			}
		}

		Thread				_thread; //the last one to be initialized
	};

	/** @} */

}


#endif
