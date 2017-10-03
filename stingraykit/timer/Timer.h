#ifndef STINGRAYKIT_TIMER_TIMER_H
#define STINGRAYKIT_TIMER_TIMER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/Final.h>
#include <stingraykit/ScopeExit.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */


	class Timer : STINGRAYKIT_FINAL(Timer), public virtual ITaskExecutor
	{
		STINGRAYKIT_NONCOPYABLE(Timer);

		static NamedLogger			s_logger;

		class CallbackInfo;
		STINGRAYKIT_DECLARE_PTR(CallbackInfo);

		class CallbackQueue;
		STINGRAYKIT_DECLARE_PTR(CallbackQueue);

	public:
		typedef function<void(const std::exception&)>	ExceptionHandler;

	private:
		std::string					_timerName;
		ExceptionHandler			_exceptionHandler;
		bool						_profileCalls;

		ElapsedTime					_monotonic;
		bool						_alive;

		CallbackQueuePtr			_queue;
		ConditionVariable			_cond;

		ThreadPtr					_worker;

	public:
		explicit Timer(const std::string& timerName, const ExceptionHandler& exceptionHandler = &Timer::DefaultExceptionHandler, bool profileCalls = true);
		virtual ~Timer();

		Token SetTimeout(const TimeDuration& timeout, const function<void()>& func);
		Token SetTimer(const TimeDuration& interval, const function<void()>& func);
		Token SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const function<void()>& func);

		virtual void AddTask(const function<void()>& task)
		{ AddTask(task, null); }

		virtual void AddTask(const function<void()>& task, const FutureExecutionTester& tester);

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		static void RemoveTask(const CallbackQueuePtr& queue, const CallbackInfoPtr& ci);

		std::string GetProfilerMessage(const function<void()>& func) const;

		void ThreadFunc();
		void ExecuteTask(const CallbackInfoPtr& ci) const;
	};
	STINGRAYKIT_DECLARE_PTR(Timer);


	class ExecutionDeferrer
	{
	private:
		TimeDuration	_timeout;
		Timer&			_timer;

		Token			_connection;
		Mutex			_connectionMutex;

		Token			_doDeferConnection;
		Mutex 			_doDeferConnectionMutex;

		Mutex			_mutex;
		bool			_cancellationActive;

	public:
		explicit ExecutionDeferrer(Timer& timer, TimeDuration timeout = TimeDuration())
			: _timeout(timeout), _timer(timer), _cancellationActive(false)
		{ }

		~ExecutionDeferrer()
		{ Cancel(); }

		TimeDuration GetTimeout() const			{ return _timeout; }
		void SetTimeout(TimeDuration timeout)	{ _timeout = timeout; }
		void Cancel()
		{
			{
				MutexLock l(_mutex);
				if (IsCancellationActive())
					return;
				SetCancellationState(true);
			}

			ScopeExitInvoker sei(bind(&ExecutionDeferrer::SetCancellationState, this, false));

			{
				MutexLock l(_doDeferConnectionMutex);
				_doDeferConnection.Reset();
			}
			{
				MutexLock l(_connectionMutex);
				_connection.Reset();
			}
		}

		// we shouldn't call Defer from deferred function!
		void Defer(const function<void()>& func)
		{
			STINGRAYKIT_CHECK(_timeout != TimeDuration(), Exception("Invalid timeout!"));
			Defer(func, _timeout);
		}

		// custom timeout version - doesn't change "default" timeout value stored in deferrer - passed timeout value corresponds to the very deferring
		void Defer(const function<void()>& func, TimeDuration timeout, optional<TimeDuration> interval = null)
		{
			MutexLock l(_doDeferConnectionMutex);
			_doDeferConnection = _timer.SetTimeout(TimeDuration(), bind(&ExecutionDeferrer::DoDefer, this, func, timeout, interval));
		}

		void DeferNoTimeout(const function<void()>& func)							{ Defer(func); }
		void DeferWithTimeout(const function<void()>& func, TimeDuration timeout)	{ Defer(func, timeout); }

	private:
		void DoDefer(const function<void()>& func, TimeDuration timeout, optional<TimeDuration> interval)
		{
			MutexLock l(_connectionMutex);
			if (interval)
				_connection = _timer.SetTimer(timeout, *interval, func);
			else
				_connection = _timer.SetTimeout(timeout, func);
		}

		void SetCancellationState(bool active)
		{ MutexLock l(_mutex); _cancellationActive = active; }

		bool IsCancellationActive() const
		{ MutexLock l(_mutex); return _cancellationActive; }
	};
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrer);


	class ExecutionDeferrerWithTimer
	{
	private:
		Timer					_timer;
		ExecutionDeferrerPtr	_impl;

	public:
		explicit ExecutionDeferrerWithTimer(const std::string& timerName, TimeDuration timeout = TimeDuration())
			: _timer(timerName)
		{ _impl = make_shared<ExecutionDeferrer>(ref(_timer), timeout); }

		~ExecutionDeferrerWithTimer()
		{ _impl.reset(); }

		TimeDuration GetTimeout() const												{ return _impl->GetTimeout(); }
		void SetTimeout(TimeDuration timeout)										{ _impl->SetTimeout(timeout); }
		void Cancel()																{ _impl->Cancel(); }
		void Defer(const function<void()>& func)									{ _impl->Defer(func); }
		void Defer(const function<void()>& func, TimeDuration timeout)				{ _impl->Defer(func, timeout); }
		void DeferNoTimeout(const function<void()>& func)							{ Defer(func); }
		void DeferWithTimeout(const function<void()>& func, TimeDuration timeout)	{ Defer(func, timeout); }
	};
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrerWithTimer);

	/** @} */

}

#endif
