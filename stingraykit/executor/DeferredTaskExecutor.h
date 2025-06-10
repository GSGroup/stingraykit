#ifndef STINGRAYKIT_EXECUTOR_DEFERREDTASKEXECUTOR_H
#define STINGRAYKIT_EXECUTOR_DEFERREDTASKEXECUTOR_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/IDeferredTaskExecutor.h>
#include <stingraykit/log/Logger.h>

#include <deque>

namespace stingray
{

	class DeferredTaskExecutor final : public virtual IDeferredTaskExecutor
	{
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

		optional<std::string>	_activeExecutor;

	public:
		explicit DeferredTaskExecutor(const std::string& name, optional<TimeDuration> profileTimeout = DefaultProfileTimeout, const ExceptionHandlerType& exceptionHandler = &DeferredTaskExecutor::DefaultExceptionHandler);

		void AddTask(const TaskType& task, const FutureExecutionTester& tester = null) override;

		void ExecuteTasks(const ICancellationToken& token) override;
		void ClearTasks() override;

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		std::string GetProfilerMessage(const TaskType& task) const;

		void ExecuteTask(const TaskPair& task) const;
	};

}

#endif
