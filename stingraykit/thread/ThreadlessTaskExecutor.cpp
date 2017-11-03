// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ThreadlessTaskExecutor.h>

#include <stingraykit/diagnostics/AsyncProfiler.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/bind.h>

namespace stingray
{

	ThreadlessTaskExecutor::ThreadlessTaskExecutor(const ExceptionHandlerType& exceptionHandler)
		: _exceptionHandler(exceptionHandler)
	{ }


	void ThreadlessTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_syncRoot);
		_queue.push_back(std::make_pair(task, tester));
	}


	void ThreadlessTaskExecutor::ExecuteTasks()
	{
		MutexLock l(_syncRoot);
		while (!_queue.empty())
		{
			optional<TaskPair> top = _queue.front();
			_queue.pop_front();

			MutexUnlock ul(l);

			try
			{
				LocalExecutionGuard guard(top->second);
				if (guard)
				{
					AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadlessTaskExecutor::GetProfilerMessage, this, ref(top->first)), 10000, AsyncProfiler::Session::NameGetterTag());
					top->first();
				}
			}
			catch(const std::exception& ex)
			{ _exceptionHandler(ex); }

			top.reset(); // destroy object with unlocked mutex to keep lock order correct
			Thread::InterruptionPoint();
		}
	}


	void ThreadlessTaskExecutor::ClearTasks()
	{
		MutexLock l(_syncRoot);
		_queue.clear();
	}


	void ThreadlessTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ Logger::Error() << "ThreadlessTaskExecutor func exception: " << ex; }


	std::string ThreadlessTaskExecutor::GetProfilerMessage(const function<void()>& func) const
	{ return StringBuilder() % get_function_name(func) % " in some ThreadlessTaskExecutor"; }

}
