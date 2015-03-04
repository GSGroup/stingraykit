#ifndef STINGRAYKIT_THREAD_THREADPOOL_H
#define STINGRAYKIT_THREAD_THREADPOOL_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <vector>

#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray
{

	class ThreadPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadPool);

		class WorkerWrapper
		{
		private:
			Mutex				_mutex;
			bool				_busy;
			ITaskExecutorPtr	_worker;

		public:
			WorkerWrapper(const std::string& name, bool profileCalls);

			bool TryAddTask(const function<void()>& task);
			bool TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester);

		private:
			void TaskWrapper(const function<void()>& task);
		};
		STINGRAYKIT_DECLARE_PTR(WorkerWrapper);

		typedef std::vector<WorkerWrapperPtr>	Workers;

	private:
		Mutex			_mutex;
		std::string		_name;
		u32				_maxThreads;
		bool			_profileCalls;
		Workers			_workers;

	public:
		ThreadPool(const std::string& name, u32 maxThreads, bool profileCalls = true);

		void Queue(const function<void()>& task);
		void Queue(const function<void()>& task, const FutureExecutionTester& tester);

	private:
		void DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc);
	};
	STINGRAYKIT_DECLARE_PTR(ThreadPool);

}

#endif
