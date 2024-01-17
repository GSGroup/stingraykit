// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
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

	const TimeDuration ThreadTaskExecutor::DefaultProfileTimeout = TimeDuration::FromSeconds(10);
	const size_t ThreadTaskExecutor::TaskCountLimit = 256 * 4;

	ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const optional<TimeDuration>& profileTimeout, const ExceptionHandlerType& exceptionHandler)
		:	_name(name),
			_profileTimeout(profileTimeout),
			_exceptionHandler(exceptionHandler),
			_worker(name, Bind(&ThreadTaskExecutor::ThreadFunc, this, _1))
	{ }


	void ThreadTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_syncRoot);
		_queue.push_back(std::make_pair(task, tester));
		_condVar.Broadcast();

		if (_queue.size() > TaskCountLimit && _queue.size() % (TaskCountLimit / 4) == 0)
			s_logger.Error() << "Task queue size limit is exceeded for executor '" << _name << "': " << _queue.size();
	}


	void ThreadTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Executor task exception: " << ex; }


	std::string ThreadTaskExecutor::GetProfilerMessage(const TaskType& task) const
	{ return StringBuilder() % get_function_name(task) % " in ThreadTaskExecutor '" % _name % "'"; }


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
			_queue.pop_front();

			MutexUnlock ul(l);

			ExecuteTask(*top);
			top.reset(); // destroy object with unlocked mutex to keep lock order correct
		}
	}


	void ThreadTaskExecutor::ExecuteTask(const TaskPair& task) const
	{
		try
		{
			LocalExecutionGuard guard(task.second);
			if (!guard)
				return;

			if (_profileTimeout)
			{
				AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), Bind(&ThreadTaskExecutor::GetProfilerMessage, this, wrap_const_ref(task.first)), *_profileTimeout, AsyncProfiler::NameGetterTag());
				task.first();
			}
			else
				task.first();
		}
		catch (const std::exception& ex)
		{ _exceptionHandler(ex); }
	}

}
