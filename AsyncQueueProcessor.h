#ifndef STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H
#define STINGRAY_TOOLKIT_ASYNCQUEUEPROCESSOR_H

#include <stingray/threads/ConditionVariable.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/function.h>
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

		Thread				_thread;
		ConditionVariable	_cond;
		Mutex				_lock;

		typedef std::list<ValueType> Queue;
		Queue				_queue;

	public:
		AsyncQueueProcessor(const std::string &name, const FunctorType &processor): _running(true), _processor(processor), _thread(name, bind(&AsyncQueueProcessor::ThreadFunc, this))
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
		void ThreadFunc()
		{
			MutexLock l(_lock);
			while(_running)
			{
				if (_queue.empty())
				{
					_cond.Wait(_lock);
					continue;
				}
				ValueType value = _queue.front();
				_queue.pop_front();

				MutexUnlock ll(l);
				_processor(value);
			}
		}
	};

}


#endif
