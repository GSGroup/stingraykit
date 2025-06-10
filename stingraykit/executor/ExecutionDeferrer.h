#ifndef STINGRAYKIT_EXECUTOR_EXECUTIONDEFERRER_H
#define STINGRAYKIT_EXECUTOR_EXECUTIONDEFERRER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/Timer.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_executor
	 * @{
	 */

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
	};
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrerWithTimer);

	/** @} */

}

#endif
