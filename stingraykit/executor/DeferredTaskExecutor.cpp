// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/DeferredTaskExecutor.h>

#include <stingraykit/diagnostics/AsyncProfiler.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/thread/ThreadNameAppender.h>
#include <stingraykit/ScopeExit.h>

namespace stingray
{

	const TimeDuration DeferredTaskExecutor::DefaultProfileTimeout = TimeDuration::FromSeconds(10);


	STINGRAYKIT_DEFINE_NAMED_LOGGER(DeferredTaskExecutor);


	DeferredTaskExecutor::DeferredTaskExecutor(const std::string& name, optional<TimeDuration> profileTimeout, const ExceptionHandlerType& exceptionHandler)
		:	_name(name),
			_profileTimeout(profileTimeout),
			_exceptionHandler(exceptionHandler)
	{ }


	void DeferredTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_syncRoot);
		_queue.emplace_back(task, tester);
	}


	void DeferredTaskExecutor::ExecuteTasks(const ICancellationToken& token)
	{
		MutexLock l(_syncRoot);

		if (_activeExecutor)
		{
			s_logger.Warning() << "[" << _name << "] Already running tasks in thread " << _activeExecutor;
			return;
		}

		const ScopeExitInvoker sei(Bind(&optional<std::string>::reset, wrap_ref(_activeExecutor)));
		_activeExecutor = Thread::GetCurrentThreadName();

		ThreadNameAppender tna(_name);

		while (!_queue.empty() && token)
		{
			optional<TaskPair> top = _queue.front();
			_queue.pop_front();

			MutexUnlock ul(l);

			ExecuteTask(*top);
			top.reset(); // destroy object with unlocked mutex to keep lock order correct

			Thread::InterruptionPoint();
		}
	}


	void DeferredTaskExecutor::ClearTasks()
	{
		MutexLock l(_syncRoot);

		if (!_queue.empty())
			s_logger.Warning() << "[" << _name << "] Clear " << _queue.size() << " tasks";

		_queue.clear();
	}


	void DeferredTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Uncaught exception:\n" << ex; }


	std::string DeferredTaskExecutor::GetProfilerMessage(const TaskType& task) const
	{ return StringBuilder() % get_function_name(task) % " in DeferredTaskExecutor '" % _name % "'"; }


	void DeferredTaskExecutor::ExecuteTask(const TaskPair& task) const
	{
		try
		{
			LocalExecutionGuard guard(task.second);
			if (!guard)
				return;

			if (_profileTimeout)
			{
				AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), Bind(&DeferredTaskExecutor::GetProfilerMessage, this, wrap_const_ref(task.first)), *_profileTimeout, AsyncProfiler::NameGetterTag());
				task.first();
			}
			else
				task.first();
		}
		catch (const std::exception& ex)
		{ _exceptionHandler(ex); }
	}

}
