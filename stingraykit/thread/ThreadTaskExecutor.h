#ifndef STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H
#define STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>

#include <queue>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class ThreadTaskExecutor : STINGRAYKIT_FINAL(ThreadTaskExecutor), public virtual ITaskExecutor
	{
		STINGRAYKIT_NONCOPYABLE(ThreadTaskExecutor);

	public:
		typedef function<void()>										TaskType;
		typedef function<void(const std::exception&)>					ExceptionHandlerType;

	private:
		typedef std::pair<TaskType, FutureExecutionTester>				TaskPair;
		typedef std::queue<TaskPair, std::deque<TaskPair> >				QueueType;

	public:
		static const TimeDuration DefaultProfileTimeout;

	private:
		static const size_t TaskCountLimit;

	private:
		static NamedLogger		s_logger;

		std::string				_name;
		optional<TimeDuration>	_profileTimeout;
		ExceptionHandlerType	_exceptionHandler;

		Mutex					_syncRoot;
		QueueType				_queue;
		ConditionVariable		_condVar;

		ThreadPtr				_worker;

	public:
		explicit ThreadTaskExecutor(const std::string& name, const optional<TimeDuration>& profileTimeout = DefaultProfileTimeout, const ExceptionHandlerType& exceptionHandler = &DefaultExceptionHandler);

		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester = null);

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		std::string GetProfilerMessage(const function<void()>& func) const;

		void ThreadFunc(const ICancellationToken& token);
		void ExecuteTask(const TaskPair& task) const;
	};
	STINGRAYKIT_DECLARE_PTR(ThreadTaskExecutor);

	/** @} */

}

#endif
