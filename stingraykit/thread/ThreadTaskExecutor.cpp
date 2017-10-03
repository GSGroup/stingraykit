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

namespace stingray
{

	STINGRAYKIT_DEFINE_NAMED_LOGGER(ThreadTaskExecutor);

	static const size_t TaskCountLimit = 256 * 4;

	ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler, bool profileCalls)
		:	_name(name),
			_exceptionHandler(exceptionHandler),
			_profileCalls(profileCalls),
			_worker(make_shared<Thread>(name, bind(&ThreadTaskExecutor::ThreadFunc, this, _1)))
	{ }


	void ThreadTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_syncRoot);
		_queue.push(std::make_pair(task, tester));
		_condVar.Broadcast();

		if (_queue.size() > TaskCountLimit && _queue.size() % (TaskCountLimit / 4) == 0)
			s_logger.Error() << "Task queue size limit is exceeded for executor '" << _name << "': " << _queue.size();
	}


	void ThreadTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Executor func exception: " << ex; }


	std::string ThreadTaskExecutor::GetProfilerMessage(const function<void()>& func) const
	{ return StringBuilder() % get_function_name(func) % " in ThreadTaskExecutor '" % _name % "'"; }


	void ThreadTaskExecutor::ThreadFunc(const ICancellationToken& token)
	{
		MutexLock l(_syncRoot);

		while (token || !_queue.empty())
		{
			if (_queue.empty())
			{
				_condVar.Wait(_syncRoot, token);
				continue;
			}

			optional<TaskPair> top = _queue.front();
			_queue.pop();

			MutexUnlock ul(l);

			ExecuteTask(*top);
			top.reset(); // destroy object with unlocked mutex to keep lock order correct
		}
	}


	void ThreadTaskExecutor::ExecuteTask(const TaskPair& task) const
	{
		LocalExecutionGuard guard(task.second);
		if (!guard)
			return;

		try
		{
			if (_profileCalls)
			{
				AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadTaskExecutor::GetProfilerMessage, this, ref(task.first)), 10000, AsyncProfiler::Session::NameGetterTag());
				task.first();
			}
			else
				task.first();
		}
		catch(const std::exception& ex)
		{ _exceptionHandler(ex); }
	}

}
