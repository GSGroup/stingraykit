#ifndef STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H
#define STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H

#include <stingray/threads/ConditionVariable.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/signals.h>
#include <list>

namespace stingray
{

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

	public:
		signal<void(bool)> OnIdle;

		AsyncQueueProcessor(const std::string &name, const FunctorType &processor):
			_running(true),
			_processor(processor),
			_idle(true),
			OnIdle(bind(&AsyncQueueProcessor::OnIdlePopulator, this, _1)),
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
			MutexLock l(_lock);
			_queue.push_front(value);
			_cond.Broadcast();
		}

		void PushBack(const ValueType &value)
		{
			MutexLock l(_lock);
			_queue.push_back(value);
			_cond.Broadcast();
		}

	private:
		void OnIdlePopulator(const function<void (bool)> & slot)
		{ slot(_idle); }

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
					continue;
				}
				SetIdle(l, false);
				ValueType value = _queue.front();
				_queue.pop_front();

				MutexUnlock ll(l);
				STINGRAY_TRY_DO_NO_LOGGER("exception in queue processor", _processor(value));
			}
		}

		Thread				_thread; //the last one to be initialized
	};

}


#endif
