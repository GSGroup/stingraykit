// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/timer/Timer.h>

#include <list>

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
		typedef std::list<CallbackInfoPtr>	Container;

	private:
		Mutex			_mutex;
		Container		_container;

	public:
		typedef Container::iterator iterator;

		inline Mutex &Sync()
		{ return _mutex; }

		inline bool IsEmpty() const
		{
			MutexLock l(_mutex);
			return _container.empty();
		}

		CallbackInfoPtr Top() const
		{
			MutexLock l(_mutex);
			return !_container.empty()? _container.front(): null; //atomically return top element
		}

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
		ElapsedTime					_timer;
		optional<TimeDuration>		_timeout;
		optional<TimeDuration>		_period;
		TaskLifeToken				_token;
		CallbackQueueWeakPtr		_queue;
		CallbackQueue::iterator		_iterator;
		bool						_iteratorIsValid;

	private:
		friend class CallbackQueue;

		void SetIterator(const CallbackQueue::iterator& it)		{ _iterator = it; _iteratorIsValid = true; }
		void ResetIterator()									{ _iteratorIsValid = false; }

		TimeDuration GetTimeBeforeTrigger() const
		{
			if (_timeout.is_initialized())
				return _timeout.get();
			if (_period.is_initialized())
				return _period.get();
			STINGRAYKIT_THROW("Timer::CallbackInfo internal error: neither _timeout nor _period is set!");
		}

	public:
		CallbackInfo(const FuncT& func, const optional<TimeDuration>& timeout, const optional<TimeDuration>& period, const CallbackQueuePtr& queue)
			: _func(func), _timeout(timeout), _period(period), _queue(queue), _iteratorIsValid(false)
		{ }

		const FuncT& GetFunc() const							{ return _func; }
		FutureExecutionTester GetExecutionTester() const 		{ return _token.GetExecutionTester(); }

		bool IsPeriodic() const									{ return _period.is_initialized(); }
		bool Triggered() const									{ return TimeDuration(_timer.ElapsedMilliseconds()) >= GetTimeBeforeTrigger(); }
		void Restart()											{ _timeout.reset(); _timer.Restart(); }
		//TimeDuration GetEstimate() const						{ TimeDuration e(_timer.ElapsedMilliseconds()); return std::max(TimeDuration(), GetTimeBeforeTrigger() - e); }
		TimeDuration GetEstimate() const						{ TimeDuration e(_timer.ElapsedMilliseconds()); return GetTimeBeforeTrigger() > e ? GetTimeBeforeTrigger() - e : TimeDuration(); }

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

	struct Timer::CallbackInfoOrder : std::binary_function<CallbackInfoPtr, CallbackInfoPtr, bool>
	{
		bool operator () (const CallbackInfoPtr& left, const CallbackInfoPtr& right) const
		{ return right->GetEstimate() < left->GetEstimate(); } // Callbacks with the least time have the greatest priority
	};

	void Timer::CallbackQueue::Push(CallbackInfoPtr ci)
	{
		MutexLock l(_mutex);
		iterator where = _container.begin();
		while (where != _container.end() && CallbackInfoOrder()(*where, ci) <= 0)
			++where;
		ci->SetIterator(_container.insert(where, ci));
	}

	void Timer::CallbackQueue::Erase(const iterator & it)
	{
		MutexLock l(_mutex);
		_container.erase(it);
	}

	Timer::CallbackInfoPtr Timer::CallbackQueue::Pop()
	{
		MutexLock l(_mutex);
		STINGRAYKIT_CHECK(!_container.empty(), "popping callback from empty list");

		CallbackInfoPtr ci = _container.front();
		_container.pop_front();
		ci->ResetIterator();
		return ci;
	}

	STINGRAYKIT_DEFINE_NAMED_LOGGER(Timer);

	Timer::Timer(const std::string& timerName, const ExceptionHandler& exceptionHandler, bool profileCalls)
		: _timerName(timerName), _working(true), _alive(true), _queue(new CallbackQueue), _exceptionHandler(exceptionHandler), _profileCalls(profileCalls)
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


	void Timer::Start()
	{
		MutexLock l(_queue->Sync());
		_working = true;
	}


	void Timer::Stop()
	{
		MutexLock l(_queue->Sync());
		_working = false;
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

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, timeout, null, _queue);
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<TimerConnectionToken>(ci);
	}


	Token Timer::SetTimer(const TimeDuration& interval, const function<void()>& func)
	{ return SetTimer(interval, interval, func); }


	Token Timer::SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const function<void()>& func)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, timeout, interval, _queue);
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<TimerConnectionToken>(ci);
	}


	void Timer::AddTask(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(MakeCancellableFunction(task, tester), TimeDuration(), null, _queue);
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

			if (top->Triggered())
			{
				top = _queue->Pop(); //fixme: check that's the same object
				bool working = _working;

				{
					LocalExecutionGuard guard(top->GetExecutionTester());
					if (!guard)
					{
						top.reset();
						continue;
					}

					MutexUnlock ul(l);
					if (top->IsPeriodic())
						top->Restart();

					if (working)
					{
						try
						{
							if (_profileCalls)
							{
								AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&Timer::GetProfilerMessage, this, ref(top->GetFunc())), 10000, AsyncProfiler::Session::Behaviour::Silent, AsyncProfiler::Session::NameGetterTag());
								(top->GetFunc())();
							}
							else
								(top->GetFunc())();
						}
						catch(const std::exception &ex)
						{ _exceptionHandler(ex); }
					}

					if (!top->IsPeriodic())
						top.reset();
				}

				if (top)
					_queue->Push(top);
			}
			else //top timer not triggered
			{
				TimeDuration wait_time = top->GetEstimate();
				top.reset();
				if (wait_time.GetMilliseconds() > 0)
					_cond.TimedWait(_queue->Sync(), wait_time);
			}
		}
	}


	void Timer::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Timer func exception: " << ex; }

}
