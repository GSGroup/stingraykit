#ifndef STINGRAY_TOOLKIT_THREADTASKEXECUTOR_H
#define STINGRAY_TOOLKIT_THREADTASKEXECUTOR_H

#include <queue>
#include <list>

#include <stingray/threads/ConditionVariable.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/ITaskExecutor.h>


namespace stingray
{


	class ThreadTaskExecutor : TOOLKIT_FINAL(ThreadTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>										TaskType;
		typedef function<void(const std::exception&)>					ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>				TaskPair;
		typedef std::queue<TaskPair, std::list<TaskPair> >				QueueType;

	private:
		std::string				_name;
		volatile bool			_working;
		volatile bool			_paused;
		ThreadPtr				_worker;
		Mutex					_syncRoot;
		ConditionVariable		_condVar;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;
		bool					_profileCalls;

	public:
		explicit ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler, bool profileCalls = true);
		~ThreadTaskExecutor();

		virtual void AddTask(const TaskType& task);
		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester);
		virtual void Pause(bool pause);

	private:
		std::string GetProfilerMessage(const function<void()>& func);
		void ThreadFunc();
	};
	TOOLKIT_DECLARE_PTR(ThreadTaskExecutor);

}


#endif
