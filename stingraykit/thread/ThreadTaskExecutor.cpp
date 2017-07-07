// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ThreadTaskExecutor.h>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function_name_getter.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	static const size_t TaskCountLimit = 1024;

	ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler, bool profileCalls)
		:	_name(name),
			_exceptionHandler(exceptionHandler),
			_profileCalls(profileCalls),
			_working(true),
			_paused(false)
	{ _worker = make_shared<Thread>(name, bind(&ThreadTaskExecutor::ThreadFunc, this, not_using(_1))); }


	ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, bool profileCalls)
		:	_name(name),
			_exceptionHandler(&ThreadTaskExecutor::DefaultExceptionHandler),
			_profileCalls(profileCalls),
			_working(true),
			_paused(false)
	{ _worker = make_shared<Thread>(name, bind(&ThreadTaskExecutor::ThreadFunc, this, not_using(_1))); }


	ThreadTaskExecutor::~ThreadTaskExecutor()
	{
		{
			MutexLock l(_syncRoot);
			_working = false;
			_condVar.Broadcast();
		}
		_worker.reset();
	}


	void ThreadTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_syncRoot);
		_queue.push(std::make_pair(task, tester));
		if (_queue.size() > TaskCountLimit)
			Logger::Error() << "[ThreadTaskExecutor] Task queue size limit exceeded for executor '" << _name << "': " << _queue.size();
		_condVar.Broadcast();
	}


	void ThreadTaskExecutor::AddTask(const TaskType& task)
	{ AddTask(task, null); }


	void ThreadTaskExecutor::Pause(bool pause)
	{
		MutexLock l(_syncRoot);
		if (_paused == pause)
			return;

		_paused = pause;
		if (!_paused)
			_condVar.Broadcast();
	}


	void ThreadTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ Logger::Error() << "Uncaught exception in ThreadTaskExecutor: " << ex; }


	std::string ThreadTaskExecutor::GetProfilerMessage(const function<void()>& func)
	{ return StringBuilder() % get_function_name(func) % " in ThreadTaskExecutor '" % _name % "'"; }


	void ThreadTaskExecutor::ThreadFunc()
	{
		MutexLock l(_syncRoot);
		while (true)
		{
			if (!_working && _queue.empty())
				return;

			if (_paused || _queue.empty())
			{
				_condVar.Wait(_syncRoot);
				continue;
			}

			optional<TaskPair> top = _queue.front();
			_queue.pop();
			try
			{
				MutexUnlock ul(l);
				LocalExecutionGuard guard(top->second);
				if (guard)
				{
					if (_profileCalls)
					{
						AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadTaskExecutor::GetProfilerMessage, this, ref(top->first)), 10000, AsyncProfiler::Session::NameGetterTag());
						top->first();
					}
					else
						top->first();
				}
				top.reset(); // destroy object with unlocked mutex to keep lock order correct
			}
			catch(const std::exception& ex)
			{ _exceptionHandler(ex); }
		}
	}

}
