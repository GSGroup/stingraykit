#ifndef STINGRAYKIT_EXECUTOR_TIMER_H
#define STINGRAYKIT_EXECUTOR_TIMER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ITaskExecutor.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_executor
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

		std::string					_name;
		optional<TimeDuration>		_profileTimeout;
		ExceptionHandler			_exceptionHandler;

		ElapsedTime					_monotonic;
		CallbackQueuePtr			_queue;
		ConditionVariable			_cond;

		ThreadPtr					_worker;

	public:
		explicit Timer(const std::string& name, optional<TimeDuration> profileTimeout = DefaultProfileTimeout, const ExceptionHandler& exceptionHandler = &DefaultExceptionHandler);
		~Timer() override;

		void AddTask(const TaskType& task, const FutureExecutionTester& tester = null) override;

		Token SetTimeout(TimeDuration timeout, const TaskType& task);
		Token SetTimer(TimeDuration interval, const TaskType& task);
		Token SetTimer(TimeDuration timeout, TimeDuration interval, const TaskType& task);

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		static void RemoveTask(const CallbackQueuePtr& queue, const CallbackInfoPtr& ci);

		std::string GetProfilerMessage(const TaskType& task) const;

		void ThreadFunc(const ICancellationToken& token);
		void ExecuteTask(const CallbackInfoPtr& ci) const;
	};
	STINGRAYKIT_DECLARE_PTR(Timer);

	/** @} */

}

#endif
