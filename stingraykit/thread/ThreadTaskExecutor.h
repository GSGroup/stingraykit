#ifndef STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H
#define STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <queue>
#include <list>

#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>
#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class ThreadTaskExecutor : STINGRAYKIT_FINAL(ThreadTaskExecutor), public virtual ITaskExecutor
	{
		STINGRAYKIT_NONCOPYABLE(ThreadTaskExecutor);

		typedef function<void()>										TaskType;
		typedef function<void(const std::exception&)>					ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>				TaskPair;
		typedef std::queue<TaskPair, std::list<TaskPair> >				QueueType;

	private:
		std::string				_name;
		bool					_working;	// TODO: get rid of it
		bool					_paused;
		ThreadPtr				_worker;	// TODO: store it by value
		Mutex					_syncRoot;
		ConditionVariable		_condVar;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;
		bool					_profileCalls;

	public:
		ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler, bool profileCalls = true);
		explicit ThreadTaskExecutor(const std::string& name, bool profileCalls = true);
		~ThreadTaskExecutor();

		virtual void AddTask(const TaskType& task);
		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester);
		virtual void Pause(bool pause);

	private:
		static void DefaultExceptionHandler(const std::exception& ex);

		std::string GetProfilerMessage(const function<void()>& func);

		void ThreadFunc();
	};
	STINGRAYKIT_DECLARE_PTR(ThreadTaskExecutor);

	/** @} */

}


#endif
