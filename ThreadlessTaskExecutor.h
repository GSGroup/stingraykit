#ifndef STINGRAY_TOOLKIT_THREADLESSTASKEXECUTOR_H
#define STINGRAY_TOOLKIT_THREADLESSTASKEXECUTOR_H


#include <queue>
#include <list>

#include <stingray/timer/AsyncProfiler.h>
#include <stingray/timer/ExecutorsProfiler.h>
#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/threads/Thread.h>


namespace stingray
{


	class ThreadlessTaskExecutor : TOOLKIT_FINAL(ThreadlessTaskExecutor), public virtual ITaskExecutor
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

		virtual void Pause(bool pause) { TOOLKIT_THROW(NotSupportedException()); }

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
					LocalExecutionGuard guard;
					if (top->second.Execute(guard))
					{
						AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&ThreadlessTaskExecutor::GetProfilerMessage, this, ref(top->first)), 10000, AsyncProfiler::Session::Behaviour::Silent, AsyncProfiler::Session::NameGetterTag());
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

	TOOLKIT_DECLARE_PTR(ThreadlessTaskExecutor);

}



#endif
