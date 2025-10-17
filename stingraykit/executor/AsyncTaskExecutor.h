#ifndef STINGRAYKIT_EXECUTOR_ASYNCTASKEXECUTOR_H
#define STINGRAYKIT_EXECUTOR_ASYNCTASKEXECUTOR_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ITaskExecutor.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>

#include <queue>

namespace stingray
{

	/**
	 * @addtogroup toolkit_executor
	 * @{
	 */

	class AsyncTaskExecutor final : public virtual ITaskExecutor
	{
		STINGRAYKIT_NONCOPYABLE(AsyncTaskExecutor);

	public:
		using ExceptionHandlerType = function<void (const std::exception&)>;

	private:
		using TaskPair = std::pair<TaskType, FutureExecutionTester>;
		using QueueType = std::deque<TaskPair>;

	public:
		static const TimeDuration DefaultProfileTimeout;

	private:
		static NamedLogger		s_logger;

		std::string				_name;
		optional<TimeDuration>	_profileTimeout;
		ExceptionHandlerType	_exceptionHandler;

		Mutex					_syncRoot;
		QueueType				_queue;
		ConditionVariable		_condVar;

		Thread					_worker;

	public:
		explicit AsyncTaskExecutor(const std::string& name, optional<TimeDuration> profileTimeout = DefaultProfileTimeout, const ExceptionHandlerType& exceptionHandler = &DefaultExceptionHandler);

		void AddTask(const TaskType& task, const FutureExecutionTester& tester = null) override;
		void AddTask(const TaskType& task, FutureExecutionTester&& tester) override;

		void AddTask(TaskType&& task, const FutureExecutionTester& tester = null) override;
		void AddTask(TaskType&& task, FutureExecutionTester&& tester) override;

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		template < typename TaskType_, typename FutureExecutionTester_ >
		void DoAddTask(TaskType_&& task, FutureExecutionTester_&& tester);

		void ThreadFunc(const ICancellationToken& token);
		void ExecuteTask(const TaskPair& task) const;

		std::string GetProfilerMessage(const TaskType& task) const;
	};

	/** @} */

}

#endif
