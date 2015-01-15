#ifndef STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H
#define STINGRAYKIT_THREAD_THREADTASKEXECUTOR_H

#include <queue>
#include <list>

#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>
#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class ThreadTaskExecutor : STINGRAYKIT_FINAL(ThreadTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>										TaskType;
		typedef function<void(const std::exception&)>					ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>				TaskPair;
		typedef std::queue<TaskPair, std::list<TaskPair> >				QueueType;

	private:
		std::string				_name;
		bool					_working;
		bool					_paused;
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
	STINGRAYKIT_DECLARE_PTR(ThreadTaskExecutor);

	/** @} */

}


#endif
