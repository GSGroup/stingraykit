// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/AsyncTaskExecutor.h>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/bind.h>

namespace stingray
{

	namespace
	{

		const size_t TaskQueueSizeWarningThreshold = 256 * 4;

	}


	const TimeDuration AsyncTaskExecutor::DefaultProfileTimeout = TimeDuration::FromSeconds(10);


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncTaskExecutor);


	AsyncTaskExecutor::AsyncTaskExecutor(const std::string& name, optional<TimeDuration> profileTimeout, const ExceptionHandlerType& exceptionHandler)
		:	_name(name),
			_profileTimeout(profileTimeout),
			_exceptionHandler(exceptionHandler),
			_worker(name, Bind(&AsyncTaskExecutor::ThreadFunc, this, _1))
	{ STINGRAYKIT_CHECK(!_profileTimeout || _profileTimeout >= TimeDuration(), ArgumentException("profileTimeout", _profileTimeout)); }


	void AsyncTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{ DoAddTask(task, tester); }


	void AsyncTaskExecutor::AddTask(const TaskType& task, FutureExecutionTester&& tester)
	{ DoAddTask(task, std::move(tester)); }


	void AsyncTaskExecutor::AddTask(TaskType&& task, const FutureExecutionTester& tester)
	{ DoAddTask(std::move(task), tester); }


	void AsyncTaskExecutor::AddTask(TaskType&& task, FutureExecutionTester&& tester)
	{ DoAddTask(std::move(task), std::move(tester)); }


	void AsyncTaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Uncaught exception:\n" << ex; }


	template < typename TaskType_, typename FutureExecutionTester_ >
	void AsyncTaskExecutor::DoAddTask(TaskType_&& task, FutureExecutionTester_&& tester)
	{
		MutexLock l(_syncRoot);
		_queue.emplace_back(std::forward<TaskType_>(task), std::forward<FutureExecutionTester_>(tester));
		_condVar.Broadcast();

		if (_queue.size() > TaskQueueSizeWarningThreshold && _queue.size() % (TaskQueueSizeWarningThreshold / 4) == 0)
			s_logger.Error() << "[" << _name << "] Too many tasks in the queue: " << _queue.size();
	}


	void AsyncTaskExecutor::ThreadFunc(const ICancellationToken& token)
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


	void AsyncTaskExecutor::ExecuteTask(const TaskPair& task) const
	{
		try
		{
			LocalExecutionGuard guard(task.second);
			if (!guard)
				return;

			if (_profileTimeout)
			{
				AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), Bind(&AsyncTaskExecutor::GetProfilerMessage, this, wrap_const_ref(task.first)), *_profileTimeout);
				task.first();
			}
			else
				task.first();
		}
		catch (const std::exception& ex)
		{ _exceptionHandler(ex); }
	}


	std::string AsyncTaskExecutor::GetProfilerMessage(const TaskType& task) const
	{ return StringBuilder() % get_function_name(task) % " in AsyncTaskExecutor '" % _name % "'"; }

}
