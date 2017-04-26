#ifndef STINGRAYKIT_THREAD_THREADLESSTASKEXECUTOR_H
#define STINGRAYKIT_THREAD_THREADLESSTASKEXECUTOR_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <queue>
#include <list>

#include <stingraykit/diagnostics/AsyncProfiler.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/Final.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/thread/Thread.h>


namespace stingray
{


	class ThreadlessTaskExecutor : STINGRAYKIT_FINAL(ThreadlessTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>							TaskType;
		typedef function<void(const std::exception&)>		ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>	TaskPair;
		typedef std::queue<TaskPair, std::list<TaskPair> >	QueueType;

	private:
		Mutex					_syncRoot;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;

	public:
		explicit ThreadlessTaskExecutor(const ExceptionHandlerType& exceptionHandler = &ITaskExecutor::DefaultExceptionHandler)
			: _exceptionHandler(exceptionHandler)
		{ }

		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester)
		{
			MutexLock l(_syncRoot);
			_queue.push(std::make_pair(task, tester));
		}

		virtual void AddTask(const TaskType& task)
		{ AddTask(task, null); }

		virtual void Pause(bool pause) { STINGRAYKIT_THROW(NotSupportedException()); }

		void ExecuteTasks()
		{
			MutexLock l(_syncRoot);
			while (!_queue.empty())
			{
				optional<TaskPair> top = _queue.front();
				_queue.pop();

				MutexUnlock ul(l);

				try
				{
					//Tracer tracer("ThreadlessTaskExecutor::ExecuteTasks: executing pending task"); //fixme: dependency to log/
					LocalExecutionGuard guard(top->second);
					if (guard)
					{
						AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadlessTaskExecutor::GetProfilerMessage, this, ref(top->first)), 10000, AsyncProfiler::Session::NameGetterTag());
						top->first();
					}
					Thread::InterruptionPoint();
				}
				catch(const std::exception& ex)
				{ _exceptionHandler(ex); }

				top.reset(); // destroy object with unlocked mutex to keep lock order correct
			}
		}

		std::string GetProfilerMessage(const function<void()>& func)
		{ return StringBuilder() % get_function_name(func) % " in some ThreadlessTaskExecutor"; }
	};

	STINGRAYKIT_DECLARE_PTR(ThreadlessTaskExecutor);

}



#endif
