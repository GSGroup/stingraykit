#ifndef STINGRAYKIT_THREAD_THREADPOOL_H
#define STINGRAYKIT_THREAD_THREADPOOL_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ConditionVariable.h>

namespace stingray
{

	class ThreadPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadPool);

	public:
		using Task = function<void (const ICancellationToken&)>;
		using ExceptionHandler = function<void (const std::exception&)>;

	private:
		class WorkerWrapper;
		using Workers = std::vector<unique_ptr<WorkerWrapper>>;

	public:
		static const TimeDuration DefaultProfileTimeout;
		static const TimeDuration DefaultIdleTimeout;

	private:
		std::string				_name;
		size_t					_maxThreads;
		optional<TimeDuration>	_profileTimeout;
		optional<TimeDuration>	_idleTimeout;
		ExceptionHandler		_exceptionHandler;

		Mutex					_mutex;
		optional<Task>			_task;
		ConditionVariable		_cond;
		ConditionVariable		_completedCond;

		Workers					_workers;
		unique_ptr<Thread>		_worker;

	public:
		ThreadPool(const std::string& name, size_t maxThreads, optional<TimeDuration> profileTimeout = DefaultProfileTimeout, optional<TimeDuration> idleTimeout = DefaultIdleTimeout, const ExceptionHandler& exceptionHandler = &DefaultExceptionHandler);
		~ThreadPool();

		bool CanQueue() const;

		void Queue(const Task& task);
		bool TryQueue(const Task& task);
		void WaitQueue(const Task& task, const ICancellationToken& token);

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		std::string GetProfilerMessage(const Task& task) const;

		void TaskCompletedHandler();

		void ThreadFunc(const ICancellationToken& token);
		void ExecuteTask(const ICancellationToken& token, const Task& task) const;
	};
	STINGRAYKIT_DECLARE_PTR(ThreadPool);

}

#endif
