#if !HAVE_TASK_EXECUTOR

#include <stingray/toolkit/log/Logger.h>
#include <stingray/timer/ExecutorsProfiler.h>
#include <stingray/toolkit/ThreadTaskExecutor.h>
#include <stingray/toolkit/function/bind.h>
#include <stingray/toolkit/function/function_name_getter.h>


namespace stingray
{

	static const size_t TaskCountLimit = 1024;

	ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler, bool profileCalls)
		: _name(name), _working(true), _paused(false), _exceptionHandler(exceptionHandler), _profileCalls(profileCalls)
	{ _worker.reset(new Thread(name, bind(&ThreadTaskExecutor::ThreadFunc, this, not_using(_1)))); }


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
				LocalExecutionGuard guard;
				if (top->second.Execute(guard))
				{
					if (_profileCalls)
					{
						AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadTaskExecutor::GetProfilerMessage, this, ref(top->first)), 10000, AsyncProfiler::Session::Behaviour::Silent, AsyncProfiler::Session::NameGetterTag());
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

#endif
