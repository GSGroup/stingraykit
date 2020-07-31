// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/timer/Timer.h>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/CancellableFunction.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/thread/TimedCancellationToken.h>
#include <stingraykit/FunctionToken.h>

#include <list>

namespace stingray
{

	class Timer::CallbackQueue
	{
		typedef std::list<CallbackInfoPtr>					ContainerInternal;
		typedef std::map<TimeDuration, ContainerInternal>	Container;

	private:
		Mutex			_mutex;
		Container		_container;

	public:
		typedef ContainerInternal::iterator iterator;

		Mutex& Sync()
		{ return _mutex; }

		bool IsEmpty() const
		{
			MutexLock l(_mutex);
			return _container.empty();
		}

		CallbackInfoPtr Top() const;
		void Push(const CallbackInfoPtr& ci);
		void Erase(const CallbackInfoPtr& ci);
		CallbackInfoPtr Pop();
	};

	class Timer::CallbackInfo
	{
		STINGRAYKIT_NONCOPYABLE(CallbackInfo);

	private:
		typedef CallbackQueue::iterator		QueueIterator;

	private:
		TaskType					_task;
		TimeDuration				_timeToTrigger;
		optional<TimeDuration>		_period;
		TaskLifeToken				_token;

		bool						_erased;
		optional<QueueIterator>		_iterator;

	private:
		friend class CallbackQueue;

		void SetIterator(const optional<QueueIterator>& it)		{ _iterator = it; }
		const optional<QueueIterator>& GetIterator() const		{ return _iterator; }
		void SetErased()										{ _erased = true; }
		bool IsErased() const									{ return _erased; }

	public:
		CallbackInfo(const TaskType& task, const TimeDuration& timeToTrigger, const optional<TimeDuration>& period, const TaskLifeToken& token)
			:	_task(task),
				_timeToTrigger(timeToTrigger),
				_period(period),
				_token(token),
				_erased(false)
		{ }

		const TaskType& GetTask() const							{ return _task; }
		FutureExecutionTester GetExecutionTester() const 		{ return _token.GetExecutionTester(); }
		void Release()											{ _token.Release(); }

		bool IsPeriodic() const									{ return _period.is_initialized(); }
		void Restart(const TimeDuration& currentTime)
		{
			STINGRAYKIT_CHECK(_period, "CallbackInfo::Restart internal error: _period is set!");
			_timeToTrigger = currentTime + *_period;
		}
		TimeDuration GetTimeToTrigger() const					{ return _timeToTrigger; }
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

	void Timer::CallbackQueue::Push(const CallbackInfoPtr& ci)
	{
		MutexLock l(_mutex);
		if (ci->IsErased())
			return;

		ContainerInternal& listToInsert = _container[ci->GetTimeToTrigger()];
		ci->SetIterator(listToInsert.insert(listToInsert.end(), ci));
	}

	void Timer::CallbackQueue::Erase(const CallbackInfoPtr& ci)
	{
		MutexLock l(_mutex);
		ci->SetErased();

		const optional<iterator>& it = ci->GetIterator();
		if (!it)
			return;

		TimeDuration keyToErase = ci->GetTimeToTrigger();
		ContainerInternal& listToErase = _container[keyToErase];
		listToErase.erase(*it);
		if (listToErase.empty())
			_container.erase(keyToErase);
		ci->SetIterator(null);
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
		ci->SetIterator(null);
		return ci;
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(Timer);

	const TimeDuration Timer::DefaultProfileTimeout = TimeDuration::FromSeconds(10);

	Timer::Timer(const std::string& timerName, const optional<TimeDuration>& profileTimeout, const ExceptionHandler& exceptionHandler)
		:	_timerName(timerName),
			_profileTimeout(profileTimeout),
			_exceptionHandler(exceptionHandler),
			_queue(make_shared_ptr<CallbackQueue>()),
			_worker(make_shared_ptr<Thread>(timerName, Bind(&Timer::ThreadFunc, this, _1)))
	{ }


	Timer::~Timer()
	{
		_worker.reset();

		MutexLock l(_queue->Sync());
		if (!_queue->IsEmpty())
			s_logger.Warning() << "killing timer " << _timerName << " which still has some functions to execute";

		while(!_queue->IsEmpty())
		{
			CallbackInfoPtr top = _queue->Pop();
			MutexUnlock ul(l);
			top.reset();
		}
	}


	Token Timer::SetTimeout(const TimeDuration& timeout, const TaskType& task)
	{
		const CallbackInfoPtr ci = make_shared_ptr<CallbackInfo>(task, _monotonic.Elapsed() + timeout, null, TaskLifeToken());
		const Token token = MakeToken<FunctionToken>(Bind(&Timer::RemoveTask, _queue, ci));

		{
			MutexLock l(_queue->Sync());
			_queue->Push(ci);
			_cond.Broadcast();
		}

		return token;
	}


	Token Timer::SetTimer(const TimeDuration& interval, const TaskType& task)
	{ return SetTimer(interval, interval, task); }


	Token Timer::SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const TaskType& task)
	{
		const CallbackInfoPtr ci = make_shared_ptr<CallbackInfo>(task, _monotonic.Elapsed() + timeout, interval, TaskLifeToken());
		const Token token = MakeToken<FunctionToken>(Bind(&Timer::RemoveTask, _queue, ci));

		{
			MutexLock l(_queue->Sync());
			_queue->Push(ci);
			_cond.Broadcast();
		}

		return token;
	}


	void Timer::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		const CallbackInfoPtr ci = make_shared_ptr<CallbackInfo>(MakeCancellableFunction(task, tester), _monotonic.Elapsed(), null, TaskLifeToken::CreateDummyTaskToken());

		MutexLock l(_queue->Sync());
		_queue->Push(ci);
		_cond.Broadcast();
	}


	void Timer::RemoveTask(const CallbackQueuePtr& queue, const CallbackInfoPtr& ci)
	{
		{
			MutexLock l(queue->Sync());
			queue->Erase(ci);
		}
		ci->Release();
	}


	void Timer::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Timer task exception: " << ex; }


	std::string Timer::GetProfilerMessage(const TaskType& task) const
	{ return StringBuilder() % get_function_name(task) % " in Timer '" % _timerName % "'"; }


	void Timer::ThreadFunc(const ICancellationToken& token)
	{
		MutexLock l(_queue->Sync());

		while (token)
		{
			if (_queue->IsEmpty())
			{
				_cond.Wait(_queue->Sync(), token);
				continue;
			}

			CallbackInfoPtr top = _queue->Top();
			if (top->GetTimeToTrigger() <= _monotonic.Elapsed())
			{
				_queue->Pop();

				MutexUnlock ul(l);

				const optional<TimeDuration> monotonic = top->IsPeriodic() ? _monotonic.Elapsed() : optional<TimeDuration>();

				ExecuteTask(top);

				if (monotonic)
				{
					top->Restart(*monotonic);
					_queue->Push(top);
				}

				top.reset();
			}
			else //top timer not triggered
			{
				const TimeDuration waitTime = top->GetTimeToTrigger() - _monotonic.Elapsed();
				top.reset();
				if (waitTime > TimeDuration())
					_cond.Wait(_queue->Sync(), TimedCancellationToken(token, waitTime));
			}
		}

		const TimeDuration currentTime = _monotonic.Elapsed();
		while (!_queue->IsEmpty())
		{
			CallbackInfoPtr top = _queue->Top();

			if (top->GetTimeToTrigger() > currentTime)
				break;

			_queue->Pop();

			MutexUnlock ul(l);

			ExecuteTask(top);
			top.reset();
		}
	}


	void Timer::ExecuteTask(const CallbackInfoPtr& ci) const
	{
		try
		{
			LocalExecutionGuard guard(ci->GetExecutionTester());
			if (!guard)
				return;

			if (_profileTimeout)
			{
				AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), Bind(&Timer::GetProfilerMessage, this, wrap_const_ref(ci->GetTask())), *_profileTimeout, AsyncProfiler::NameGetterTag());
				ci->GetTask()();
			}
			else
				ci->GetTask()();
		}
		catch(const std::exception &ex)
		{ _exceptionHandler(ex); }
	}


	////////////////////////////////////////////////////////////////////////////////


	void ExecutionDeferrer::Cancel()
	{
		{
			MutexLock l(_mutex);
			if (_cancellationActive)
				return;
			_cancellationActive = true;
		}

		{
			MutexLock l(_doDeferConnectionMutex);
			_doDeferConnection.Reset();
		}
		{
			MutexLock l(_connectionMutex);
			_connection.Reset();
		}

		MutexLock l(_mutex);
		_cancellationActive = false;
	}


	void ExecutionDeferrer::Defer(const function<void()>& func)
	{
		STINGRAYKIT_CHECK(_timeout != TimeDuration(), Exception("Invalid timeout!"));
		Defer(func, _timeout);
	}


	void ExecutionDeferrer::Defer(const function<void()>& func, TimeDuration timeout, optional<TimeDuration> interval)
	{
		MutexLock l(_doDeferConnectionMutex);
		_doDeferConnection = _timer.SetTimeout(TimeDuration(), Bind(&ExecutionDeferrer::DoDefer, this, func, timeout, interval));
	}


	void ExecutionDeferrer::DoDefer(const function<void()>& func, TimeDuration timeout, optional<TimeDuration> interval)
	{
		MutexLock l(_connectionMutex);
		if (interval)
			_connection = _timer.SetTimer(timeout, *interval, func);
		else
			_connection = _timer.SetTimeout(timeout, func);
	}

}
