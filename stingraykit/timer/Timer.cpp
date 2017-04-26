// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/timer/Timer.h>

#include <list>
#include <map>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/CancellableFunction.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function_name_getter.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/TaskLifeToken.h>


namespace stingray
{

	namespace Detail
	{
		struct ITimerConnectionImpl
		{
			virtual ~ITimerConnectionImpl() { }

			virtual void Disconnect() = 0;
		};
		STINGRAYKIT_DECLARE_PTR(ITimerConnectionImpl);
	}


	class TimerConnectionToken : public virtual IToken
	{
		friend class Timer;

	private:
		Detail::ITimerConnectionImplPtr		_impl;

	public:
		TimerConnectionToken(const Detail::ITimerConnectionImplPtr& impl) : _impl(impl)
		{ }

		~TimerConnectionToken()
		{
			if (_impl)
			{
				_impl->Disconnect();
				_impl.reset();
			}
		}
	};


	class Timer::CallbackQueue
	{
		typedef std::list<CallbackInfoPtr>					ContainerInternal;
		typedef std::map<TimeDuration, ContainerInternal>	Container;

	private:
		Mutex			_mutex;
		Container		_container;

	public:
		typedef ContainerInternal::iterator iterator;

		inline Mutex &Sync()
		{ return _mutex; }

		inline bool IsEmpty() const
		{
			MutexLock l(_mutex);
			return _container.empty();
		}

		CallbackInfoPtr Top() const;
		void Push(CallbackInfoPtr ci);
		void Erase(const iterator & it);
		CallbackInfoPtr Pop();
	};

	class Timer::CallbackInfo : public virtual Detail::ITimerConnectionImpl
	{
		STINGRAYKIT_NONCOPYABLE(CallbackInfo);
		typedef function<void()>		FuncT;

	private:
		FuncT						_func;
		TimeDuration				_timeToTrigger;
		optional<TimeDuration>		_period;
		TaskLifeToken				_token;
		CallbackQueueWeakPtr		_queue;
		CallbackQueue::iterator		_iterator;
		bool						_iteratorIsValid;

	private:
		friend class CallbackQueue;

		void SetIterator(const CallbackQueue::iterator& it)		{ _iterator = it; _iteratorIsValid = true; }
		void ResetIterator()									{ _iteratorIsValid = false; }

	public:
		CallbackInfo(const FuncT& func, const TimeDuration& timeToTrigger, const optional<TimeDuration>& period, const CallbackQueuePtr& queue)
			: _func(func), _timeToTrigger(timeToTrigger), _period(period), _queue(queue), _iteratorIsValid(false)
		{ }

		const FuncT& GetFunc() const							{ return _func; }
		FutureExecutionTester GetExecutionTester() const 		{ return _token.GetExecutionTester(); }

		bool IsPeriodic() const									{ return _period.is_initialized(); }
		void Restart(const TimeDuration& currentTime)
		{
			STINGRAYKIT_CHECK(_period, "Timer::CallbackInfo::Restart internal error: _period is set!");
			_timeToTrigger = currentTime + *_period;
		}
		TimeDuration GetTimeToTrigger() const					{ return _timeToTrigger; }

		virtual void Disconnect()
		{
			CallbackQueuePtr qm = _queue.lock();
			if (qm)
			{
				MutexLock l(qm->Sync());
				if (_iteratorIsValid)
				{
					qm->Erase(_iterator);
					ResetIterator();
				}
			}
			_token.Release();
		}
	};

	Timer::CallbackInfoPtr Timer::CallbackQueue::Top() const
	{
		MutexLock l(_mutex);
		if (!_container.empty())
		{
			const ContainerInternal& listForTop = _container.begin()->second;
			STINGRAYKIT_CHECK(!listForTop.empty(), "try to get callback from empty list");
			return listForTop.front();
		}
		else
			return null;
	}

	void Timer::CallbackQueue::Push(CallbackInfoPtr ci)
	{
		MutexLock l(_mutex);
		ContainerInternal& listToInsert = _container[ci->GetTimeToTrigger()];
		ci->SetIterator(listToInsert.insert(listToInsert.end(), ci));
	}

	void Timer::CallbackQueue::Erase(const iterator & it)
	{
		MutexLock l(_mutex);
		TimeDuration keyToErase = (*it)->GetTimeToTrigger();
		ContainerInternal& listToErase = _container[keyToErase];
		listToErase.erase(it);
		if (listToErase.empty())
			_container.erase(keyToErase);
	}

	Timer::CallbackInfoPtr Timer::CallbackQueue::Pop()
	{
		MutexLock l(_mutex);
		STINGRAYKIT_CHECK(!_container.empty(), "popping callback from empty map");
		ContainerInternal& listToPop = _container.begin()->second;
		STINGRAYKIT_CHECK(!listToPop.empty(), "popping callback from empty list");

		CallbackInfoPtr ci = listToPop.front();
		listToPop.pop_front();
		if (listToPop.empty())
			_container.erase(ci->GetTimeToTrigger());
		ci->ResetIterator();
		return ci;
	}

	STINGRAYKIT_DEFINE_NAMED_LOGGER(Timer);

	Timer::Timer(const std::string& timerName, const ExceptionHandler& exceptionHandler, bool profileCalls) :
		_timerName(timerName), _alive(true), _queue(new CallbackQueue), _exceptionHandler(exceptionHandler), _profileCalls(profileCalls)
	{
		_worker.reset(new Thread(timerName, bind(&Timer::ThreadFunc, this, not_using(_1))));
	}


	Timer::~Timer()
	{
		Shutdown();

		bool has_tasks = false;
		while(!_queue->IsEmpty())
		{
			CallbackInfoPtr i = _queue->Pop();
			LocalExecutionGuard guard(i->GetExecutionTester());
			if (guard)
			{
				has_tasks = true;
				break;
			}
		}

		if (has_tasks)
			Logger::Warning() << "[Timer] Killing timer " << _timerName << " which still has some functions to execute";
	}


	void Timer::Shutdown()
	{
		{
			MutexLock l(_queue->Sync());
			_alive = false;
			_cond.Broadcast();
		}
		_worker.reset();
	}


	Token Timer::SetTimeout(const TimeDuration& timeout, const function<void()>& func)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, _monotonic.Elapsed() + timeout, null, _queue);
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<TimerConnectionToken>(ci);
	}


	Token Timer::SetTimer(const TimeDuration& interval, const function<void()>& func)
	{ return SetTimer(interval, interval, func); }


	Token Timer::SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const function<void()>& func)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, _monotonic.Elapsed() + timeout, interval, _queue);
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<TimerConnectionToken>(ci);
	}


	void Timer::AddTask(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(MakeCancellableFunction(task, tester), _monotonic.Elapsed(), null, _queue);
		_queue->Push(ci);
		_cond.Broadcast();
	}


	std::string Timer::GetProfilerMessage(const function<void()>& func)
	{ return StringBuilder() % get_function_name(func) % " in Timer '" % _timerName % "'"; }


	void Timer::ThreadFunc()
	{
		MutexLock l(_queue->Sync());

		while (_alive)
		{
			CallbackInfoPtr top = _queue->Top();
			if (!top)
			{
				_cond.Wait(_queue->Sync());
				continue;
			}

			TimeDuration wait_time = top->GetTimeToTrigger() - _monotonic.Elapsed();
			if (wait_time.GetMilliseconds() <= 0)
			{
				top = _queue->Pop(); //fixme: check that's the same object

				{
					LocalExecutionGuard guard(top->GetExecutionTester());
					if (!guard)
					{
						top.reset();
						continue;
					}

					MutexUnlock ul(l);
					if (top->IsPeriodic())
						top->Restart(_monotonic.Elapsed());

					try
					{
						if (_profileCalls)
						{
							AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&Timer::GetProfilerMessage, this, ref(top->GetFunc())), 10000, AsyncProfiler::Session::NameGetterTag());
							(top->GetFunc())();
						}
						else
							(top->GetFunc())();
					}
					catch(const std::exception &ex)
					{ _exceptionHandler(ex); }

					if (!top->IsPeriodic())
						top.reset();
				}

				if (top)
					_queue->Push(top);
			}
			else //top timer not triggered
			{
				TimeDuration wait_time = top->GetTimeToTrigger() - _monotonic.Elapsed();
				top.reset();
				if (wait_time.GetMilliseconds() > 0)
					_cond.TimedWait(_queue->Sync(), wait_time);
			}
		}

		TimeDuration currentTime = _monotonic.Elapsed();
		while (!_queue->IsEmpty())
		{
			CallbackInfoPtr top = _queue->Pop();

			TimeDuration wait_time = top->GetTimeToTrigger() - currentTime;
			if (wait_time.GetMilliseconds() <= 0)
			{
				LocalExecutionGuard guard(top->GetExecutionTester());
				if (!guard)
					continue;

				MutexUnlock ul(l);
				try
				{
					if (_profileCalls)
					{
						AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&Timer::GetProfilerMessage, this, ref(top->GetFunc())), 10000, AsyncProfiler::Session::NameGetterTag());
						(top->GetFunc())();
					}
					else
						(top->GetFunc())();
				}
				catch(const std::exception &ex)
				{ _exceptionHandler(ex); }
			}
			else
				break;
		}
	}


	void Timer::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Timer func exception: " << ex; }

}
