#ifndef STINGRAYKIT_TIMER_TIMER_H
#define STINGRAYKIT_TIMER_TIMER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ITaskExecutor.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */

	class Timer final : public virtual ITaskExecutor
	{
		STINGRAYKIT_NONCOPYABLE(Timer);

	public:
		using ExceptionHandler = function<void (const std::exception&)>;

	private:
		class CallbackInfo;
		STINGRAYKIT_DECLARE_PTR(CallbackInfo);

		class CallbackQueue;
		STINGRAYKIT_DECLARE_PTR(CallbackQueue);

	public:
		static const TimeDuration DefaultProfileTimeout;

	private:
		static NamedLogger			s_logger;

		std::string					_timerName;
		optional<TimeDuration>		_profileTimeout;
		ExceptionHandler			_exceptionHandler;

		ElapsedTime					_monotonic;
		CallbackQueuePtr			_queue;
		ConditionVariable			_cond;

		ThreadPtr					_worker;

	public:
		explicit Timer(const std::string& timerName, optional<TimeDuration> profileTimeout = DefaultProfileTimeout, const ExceptionHandler& exceptionHandler = &DefaultExceptionHandler);
		~Timer() override;

		Token SetTimeout(const TimeDuration& timeout, const TaskType& task);
		Token SetTimer(const TimeDuration& interval, const TaskType& task);
		Token SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const TaskType& task);

		void AddTask(const TaskType& task, const FutureExecutionTester& tester = null) override;

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		static void RemoveTask(const CallbackQueuePtr& queue, const CallbackInfoPtr& ci);

		std::string GetProfilerMessage(const TaskType& task) const;

		void ThreadFunc(const ICancellationToken& token);
		void ExecuteTask(const CallbackInfoPtr& ci) const;
	};
	STINGRAYKIT_DECLARE_PTR(Timer);


	class ExecutionDeferrer
	{
	public:
		using TaskType = Timer::TaskType;

	private:
		Timer&			_timer;
		TimeDuration	_timeout;

		Mutex			_mutex;
		bool			_cancellationActive;

		Mutex			_connectionMutex;
		Token			_connection;

		Mutex 			_doDeferConnectionMutex;
		Token			_doDeferConnection;

	public:
		explicit ExecutionDeferrer(Timer& timer, TimeDuration timeout = TimeDuration())
			: _timer(timer), _timeout(timeout), _cancellationActive(false)
		{ }

		void Cancel();

		// we shouldn't call Defer from deferred function!
		void Defer(const TaskType& task);

		// custom timeout version - doesn't change "default" timeout value stored in deferrer - passed timeout value corresponds to the very deferring
		void Defer(const TaskType& task, TimeDuration timeout, optional<TimeDuration> interval = null);

		void DeferNoTimeout(const TaskType& task)									{ Defer(task); }
		void DeferWithTimeout(const TaskType& task, TimeDuration timeout)			{ Defer(task, timeout); }

	private:
		void DoDefer(const TaskType& task, TimeDuration timeout, optional<TimeDuration> interval);
	};
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrer);


	class ExecutionDeferrerWithTimer
	{
	public:
		using TaskType = ExecutionDeferrer::TaskType;

	private:
		Timer					_timer;
		ExecutionDeferrerPtr	_impl;

	public:
		explicit ExecutionDeferrerWithTimer(const std::string& timerName, TimeDuration timeout = TimeDuration())
			: _timer(timerName)
		{ _impl = make_shared_ptr<ExecutionDeferrer>(_timer, timeout); }

		void Cancel()																{ _impl->Cancel(); }

		void Defer(const TaskType& task)											{ _impl->Defer(task); }
		void Defer(const TaskType& task, TimeDuration timeout)						{ _impl->Defer(task, timeout); }

		void DeferNoTimeout(const TaskType& task)									{ Defer(task); }
		void DeferWithTimeout(const TaskType& task, TimeDuration timeout)			{ Defer(task, timeout); }
	};
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrerWithTimer);

	/** @} */

}

#endif
